#include "json.h"
#include "typed_point.h"


#include <library/cpp/monlib/exception/exception.h>
#include <library/cpp/monlib/metrics/labels.h>
#include <library/cpp/monlib/metrics/metric_value.h>

#include <library/cpp/json/json_reader.h>

#include <library/cpp/string_builder/string_builder.h>

#include <util/datetime/base.h>
#include <util/string/cast.h>

#include <limits>

namespace NMonitoring {

#define DECODE_ENSURE(COND, ...) MONLIB_ENSURE_EX(COND, TJsonDecodeError() << __VA_ARGS__)

namespace {

///////////////////////////////////////////////////////////////////////
// THistogramBuilder
///////////////////////////////////////////////////////////////////////
class THistogramBuilder {
public:
    void AddBound(TBucketBound bound) {
        if (!Bounds_.empty()) {
            DECODE_ENSURE(Bounds_.back() < bound,
                     "non sorted bounds, " << Bounds_.back() <<
                     " >= " << bound);
        }
        Bounds_.push_back(bound);
    }

    void AddValue(TBucketValue value) {
        Values_.push_back(value);
    }

    void AddInf(TBucketValue value) {
        InfPresented_ = true;
        InfValue_ = value;
    }

    IHistogramSnapshotPtr Build() {
        if (InfPresented_) {
            Bounds_.push_back(Max<TBucketBound>());
            Values_.push_back(InfValue_);
        }

        auto snapshot = ExplicitHistogramSnapshot(Bounds_, Values_, true);

        Bounds_.clear();
        Values_.clear();
        InfPresented_ = false;

        return snapshot;
    }

    bool Empty() const noexcept {
        return Bounds_.empty() && Values_.empty();
    }

    void Clear() {
        Bounds_.clear();
        Values_.clear();
    }

private:
    TBucketBounds Bounds_;
    TBucketValues Values_;

    bool InfPresented_ = false;
    TBucketValue InfValue_;
};

class TSummaryDoubleBuilder {
public:
    ISummaryDoubleSnapshotPtr Build() const {
        return MakeIntrusive<TSummaryDoubleSnapshot>(Sum_, Min_, Max_, Last_, Count_);
    }

    void SetSum(double sum) {
        Empty_ = false;
        Sum_ = sum;
    }

    void SetMin(double min) {
        Empty_ = false;
        Min_ = min;
    }

    void SetMax(double max) {
        Empty_ = false;
        Max_ = max;
    }

    void SetLast(double last) {
        Empty_ = false;
        Last_ = last;
    }

    void SetCount(ui64 count) {
        Empty_ = false;
        Count_ = count;
    }

    void Clear() {
        Empty_ = true;
        Sum_ = 0;
        Min_ = 0;
        Max_ = 0;
        Last_ = 0;
        Count_ = 0;
    }

    bool Empty() const {
        return Empty_;
    }

private:
    double Sum_ = 0;
    double Min_ = 0;
    double Max_ = 0;
    double Last_ = 0;
    ui64 Count_ = 0;
    bool Empty_ = true;
};

class TLogHistogramBuilder {
public:
    void SetBase(double base) {
        DECODE_ENSURE(base > 0, "base must be positive");
        Base_ = base;
    }

    void SetZerosCount(ui64 zerosCount) {
        ZerosCount_ = zerosCount;
    }

    void SetStartPower(int startPower) {
        StartPower_ = startPower;
    }

    void AddBucketValue(double value) {
        DECODE_ENSURE(value > 0.0, "bucket values must be positive");
        DECODE_ENSURE(value < std::numeric_limits<double>::max(), "bucket values must be finite");
        Buckets_.push_back(value);
    }

    void Clear() {
        Buckets_.clear();
        Base_ = 1.5;
        ZerosCount_ = 0;
        StartPower_ = 0;
    }

    bool Empty() const {
        return Buckets_.empty() && ZerosCount_ == 0;
    }

    TLogHistogramSnapshotPtr Build() {
        return MakeIntrusive<TLogHistogramSnapshot>(Base_, ZerosCount_, StartPower_, std::move(Buckets_));
    }

private:
    double Base_ = 1.5;
    ui64 ZerosCount_ = 0;
    int StartPower_ = 0;
    std::vector<double> Buckets_;
};

std::pair<double, bool> ParseSpecDouble(std::string_view string) {
    if (string == std::string_view("nan") || string == std::string_view("NaN")) {
        return {std::numeric_limits<double>::quiet_NaN(), true};
    } else if (string == std::string_view("inf") || string == std::string_view("Infinity")) {
        return {std::numeric_limits<double>::infinity(), true};
    } else if (string == std::string_view("-inf") || string == std::string_view("-Infinity")) {
        return  {-std::numeric_limits<double>::infinity(), true};
    } else {
        return {0, false};
    }
}

///////////////////////////////////////////////////////////////////////
// TMetricCollector
///////////////////////////////////////////////////////////////////////
struct TMetricCollector {
    EMetricType Type = EMetricType::UNKNOWN;
    TLabels Labels;
    THistogramBuilder HistogramBuilder;
    TSummaryDoubleBuilder SummaryBuilder;
    TLogHistogramBuilder LogHistBuilder;
    TTypedPoint LastPoint;
    std::vector<TTypedPoint> TimeSeries;

    bool SeenTsOrValue = false;
    bool SeenTimeseries = false;

    void Clear() {
        Type = EMetricType::UNKNOWN;
        Labels.Clear();
        SeenTsOrValue = false;
        SeenTimeseries = false;
        TimeSeries.clear();
        LastPoint = {};
        HistogramBuilder.Clear();
        SummaryBuilder.Clear();
        LogHistBuilder.Clear();
    }

    void AddLabel(const TLabel& label) {
        Labels.Add(label.Name(), label.Value());
    }

    void SetLastTime(TInstant time) {
        LastPoint.SetTime(time);
    }

    template <typename T>
    void SetLastValue(T value) {
        LastPoint.SetValue(value);
    }

    void SaveLastPoint() {
        DECODE_ENSURE(LastPoint.GetTime() != TInstant::Zero(),
                 "cannot add point without or zero timestamp");
        if (!HistogramBuilder.Empty()) {
            auto histogram = HistogramBuilder.Build();
            TimeSeries.emplace_back(LastPoint.GetTime(), histogram.Get());
        } else if (!SummaryBuilder.Empty()) {
            auto summary = SummaryBuilder.Build();
            TimeSeries.emplace_back(LastPoint.GetTime(), summary.Get());
        } else if (!LogHistBuilder.Empty()) {
            auto logHist = LogHistBuilder.Build();
            TimeSeries.emplace_back(LastPoint.GetTime(), logHist.Get());
        } else {
            TimeSeries.push_back(std::move(LastPoint));
        }
    }

    template <typename TConsumer>
    void Consume(TConsumer&& consumer) {
        if (TimeSeries.empty()) {
            const auto& p = LastPoint;
            consumer(p.GetTime(), p.GetValueType(), p.GetValue());
        } else {
            for (const auto& p: TimeSeries) {
                consumer(p.GetTime(), p.GetValueType(), p.GetValue());
            }
        }
    }
};

struct TCommonParts {
    TInstant CommonTime;
    TLabels CommonLabels;
};

class IHaltableMetricConsumer: public IMetricConsumer {
public:
    virtual bool NeedToStop() const = 0;
};

// TODO(ivanzhukov@): check all states for cases when a json document is invalid
//  e.g. "metrics" or "commonLabels" keys are specified multiple times
class TCommonPartsCollector: public IHaltableMetricConsumer {
public:
    TCommonParts&& CommonParts() {
        return std::move(CommonParts_);
    }

private:
    bool NeedToStop() const override {
        return TInstant::Zero() != CommonParts_.CommonTime && !CommonParts_.CommonLabels.Empty();
    }

    void OnStreamBegin() override {
    }

    void OnStreamEnd() override {
    }

    void OnCommonTime(TInstant time) override {
        CommonParts_.CommonTime = time;
    }

    void OnMetricBegin(EMetricType) override {
        IsMetric_ = true;
    }

    void OnMetricEnd() override {
        IsMetric_ = false;
    }

    void OnLabelsBegin() override {
    }

    void OnLabelsEnd() override {
    }

    void OnLabel(std::string_view name, std::string_view value) override {
        if (!IsMetric_) {
            CommonParts_.CommonLabels.Add(std::move(name), std::move(value));
        }
    }

    void OnDouble(TInstant, double) override {
    }

    void OnInt64(TInstant, i64) override {
    }

    void OnUint64(TInstant, ui64) override {
    }

    void OnHistogram(TInstant, IHistogramSnapshotPtr) override {
    }

    void OnLogHistogram(TInstant, TLogHistogramSnapshotPtr) override {
    }

    void OnSummaryDouble(TInstant, ISummaryDoubleSnapshotPtr) override {
    }

private:
    TCommonParts CommonParts_;
    bool IsMetric_{false};
};

class TCommonPartsProxy: public IHaltableMetricConsumer {
public:
    TCommonPartsProxy(TCommonParts&& commonParts, IMetricConsumer* c)
        : CommonParts_{std::move(commonParts)}
        , Consumer_{c}
    {}

private:
    bool NeedToStop() const override {
        return false;
    }

    void OnStreamBegin() override {
        Consumer_->OnStreamBegin();

        if (!CommonParts_.CommonLabels.Empty()) {
            Consumer_->OnLabelsBegin();

            for (auto&& label : CommonParts_.CommonLabels) {
                Consumer_->OnLabel(label.Name(), label.Value());
            }

            Consumer_->OnLabelsEnd();
        }

        if (TInstant::Zero() != CommonParts_.CommonTime) {
            Consumer_->OnCommonTime(CommonParts_.CommonTime);
        }
    }

    void OnStreamEnd() override {
        Consumer_->OnStreamEnd();
    }

    void OnCommonTime(TInstant) override {
    }

    void OnMetricBegin(EMetricType type) override {
        IsMetric_ = true;

        Consumer_->OnMetricBegin(type);
    }

    void OnMetricEnd() override {
        IsMetric_ = false;

        Consumer_->OnMetricEnd();
    }

    void OnLabelsBegin() override {
        if (IsMetric_) {
            Consumer_->OnLabelsBegin();
        }
    }

    void OnLabelsEnd() override {
        if (IsMetric_) {
            Consumer_->OnLabelsEnd();
        }
    }

    void OnLabel(std::string_view name, std::string_view value) override {
        if (IsMetric_) {
            Consumer_->OnLabel(std::move(name), std::move(value));
        }
    }

    void OnDouble(TInstant time, double value) override {
        Consumer_->OnDouble(time, value);
    }

    void OnInt64(TInstant time, i64 value) override {
        Consumer_->OnInt64(time, value);
    }

    void OnUint64(TInstant time, ui64 value) override {
        Consumer_->OnUint64(time, value);
    }

    void OnHistogram(TInstant time, IHistogramSnapshotPtr snapshot) override {
        Consumer_->OnHistogram(time, std::move(snapshot));
    }

    void OnLogHistogram(TInstant time, TLogHistogramSnapshotPtr snapshot) override {
        Consumer_->OnLogHistogram(time, std::move(snapshot));
    }

    void OnSummaryDouble(TInstant time, ISummaryDoubleSnapshotPtr snapshot) override {
        Consumer_->OnSummaryDouble(time, std::move(snapshot));
    }

private:
    const TCommonParts CommonParts_;
    IMetricConsumer* Consumer_;
    bool IsMetric_{false};
};

///////////////////////////////////////////////////////////////////////
// TDecoderJson
///////////////////////////////////////////////////////////////////////
class TDecoderJson final: public NJson::TJsonCallbacks {
    struct TState {
        enum EState {
            ROOT_OBJECT = 0x01,

            COMMON_LABELS,
            COMMON_TS,
            METRICS_ARRAY,

            METRIC_OBJECT,
            METRIC_NAME,
            METRIC_LABELS,
            METRIC_TYPE,
            METRIC_MODE, // TODO: must be deleted
            METRIC_TIMESERIES,
            METRIC_TS,
            METRIC_VALUE,
            METRIC_HIST,
            METRIC_HIST_BOUNDS,
            METRIC_HIST_BUCKETS,
            METRIC_HIST_INF,
            METRIC_DSUMMARY,
            METRIC_DSUMMARY_SUM,
            METRIC_DSUMMARY_MIN,
            METRIC_DSUMMARY_MAX,
            METRIC_DSUMMARY_LAST,
            METRIC_DSUMMARY_COUNT,
            METRIC_LOG_HIST,
            METRIC_LOG_HIST_BASE,
            METRIC_LOG_HIST_ZEROS,
            METRIC_LOG_HIST_START_POWER,
            METRIC_LOG_HIST_BUCKETS,
        };

        constexpr EState Current() const noexcept {
            return static_cast<EState>(State_ & 0xFF);
        }

        void ToNext(EState state) noexcept {
            constexpr auto bitSize = 8 * sizeof(ui8);
            State_ = (State_ << bitSize) | static_cast<ui8>(state);
        }

        void ToPrev() noexcept {
            constexpr auto bitSize = 8 * sizeof(ui8);
            State_ = State_ >> bitSize;
        }

    private:
        ui64 State_ = static_cast<ui64>(ROOT_OBJECT);
    };

public:
    TDecoderJson(std::string_view data, IHaltableMetricConsumer* metricConsumer, std::string_view metricNameLabel)
        : Data_(data)
        , MetricConsumer_(metricConsumer)
        , MetricNameLabel_(metricNameLabel)
    {
    }

private:
#define PARSE_ENSURE(CONDITION, ...)                     \
do {                                                 \
if (Y_UNLIKELY(!(CONDITION))) {                  \
    ErrorMsg_ = NUtils::TYdbStringBuilder() << __VA_ARGS__; \
    return false;                                \
}                                                \
} while (false)

    bool OnInteger(long long value) override {
        switch (State_.Current()) {
            case TState::COMMON_TS:
                PARSE_ENSURE(value >= 0, "unexpected negative number in a common timestamp: " << value);
                MetricConsumer_->OnCommonTime(TInstant::Seconds(value));
                State_.ToPrev();

                if (MetricConsumer_->NeedToStop()) {
                    IsIntentionallyHalted_ = true;
                    return false;
                }

                break;

            case TState::METRIC_TS:
                PARSE_ENSURE(value >= 0, "unexpected negative number in a metric timestamp: " << value);
                LastMetric_.SetLastTime(TInstant::Seconds(value));
                State_.ToPrev();
                break;

            case TState::METRIC_VALUE:
                LastMetric_.SetLastValue(static_cast<i64>(value));
                State_.ToPrev();
                break;

            case TState::METRIC_HIST_BOUNDS:
                LastMetric_.HistogramBuilder.AddBound(static_cast<double>(value));
                break;

            case TState::METRIC_HIST_BUCKETS:
                PARSE_ENSURE(value >= 0 && static_cast<ui64>(value) <= Max<TBucketValues::value_type>(), "value is out of bounds " << value);
                LastMetric_.HistogramBuilder.AddValue(value);
                break;

            case TState::METRIC_HIST_INF:
                PARSE_ENSURE(value >= 0, "unexpected negative number in histogram inf: " << value);
                LastMetric_.HistogramBuilder.AddInf(value);
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_COUNT:
                LastMetric_.SummaryBuilder.SetCount(value);
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_SUM:
                LastMetric_.SummaryBuilder.SetSum(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_MIN:
                LastMetric_.SummaryBuilder.SetMin(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_MAX:
                LastMetric_.SummaryBuilder.SetMax(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_LAST:
                LastMetric_.SummaryBuilder.SetLast(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_BASE:
                LastMetric_.LogHistBuilder.SetBase(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_ZEROS:
                LastMetric_.LogHistBuilder.SetZerosCount(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_START_POWER:
                LastMetric_.LogHistBuilder.SetStartPower(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_BUCKETS:
                LastMetric_.LogHistBuilder.AddBucketValue(value);
                break;

            default:
                return false;
        }
        return true;
    }

    bool OnUInteger(unsigned long long value) override {
        switch (State_.Current()) {
            case TState::COMMON_TS:
                MetricConsumer_->OnCommonTime(TInstant::Seconds(value));
                State_.ToPrev();

                if (MetricConsumer_->NeedToStop()) {
                    IsIntentionallyHalted_ = true;
                    return false;
                }

                break;

            case TState::METRIC_TS:
                LastMetric_.SetLastTime(TInstant::Seconds(value));
                State_.ToPrev();
                break;

            case TState::METRIC_VALUE:
                PARSE_ENSURE(value <= Max<ui64>(), "Metric value is out of bounds: " << value);
                LastMetric_.SetLastValue(static_cast<ui64>(value));
                State_.ToPrev();
                break;

            case TState::METRIC_HIST_BOUNDS:
                LastMetric_.HistogramBuilder.AddBound(static_cast<double>(value));
                break;

            case TState::METRIC_HIST_BUCKETS:
                PARSE_ENSURE(value <= Max<TBucketValues::value_type>(), "Histogram bucket value is out of bounds: " << value);
                LastMetric_.HistogramBuilder.AddValue(value);
                break;

            case TState::METRIC_HIST_INF:
                LastMetric_.HistogramBuilder.AddInf(value);
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_COUNT:
                LastMetric_.SummaryBuilder.SetCount(value);
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_SUM:
                LastMetric_.SummaryBuilder.SetSum(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_MIN:
                LastMetric_.SummaryBuilder.SetMin(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_MAX:
                LastMetric_.SummaryBuilder.SetMax(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_LAST:
                LastMetric_.SummaryBuilder.SetLast(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_BASE:
                LastMetric_.LogHistBuilder.SetBase(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_ZEROS:
                LastMetric_.LogHistBuilder.SetZerosCount(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_START_POWER:
                LastMetric_.LogHistBuilder.SetStartPower(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_BUCKETS:
                LastMetric_.LogHistBuilder.AddBucketValue(value);
                break;

            default:
                return false;
        }
        return true;
    }

    bool OnDouble(double value) override {
        switch (State_.Current()) {
            case TState::METRIC_VALUE:
                LastMetric_.SetLastValue(value);
                State_.ToPrev();
                break;

            case TState::METRIC_HIST_BOUNDS:
                LastMetric_.HistogramBuilder.AddBound(value);
                break;

            case TState::METRIC_DSUMMARY_SUM:
                LastMetric_.SummaryBuilder.SetSum(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_MIN:
                LastMetric_.SummaryBuilder.SetMin(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_MAX:
                LastMetric_.SummaryBuilder.SetMax(value);
                State_.ToPrev();
                break;
            case TState::METRIC_DSUMMARY_LAST:
                LastMetric_.SummaryBuilder.SetLast(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_BASE:
                LastMetric_.LogHistBuilder.SetBase(value);
                State_.ToPrev();
                break;

            case TState::METRIC_LOG_HIST_BUCKETS:
                LastMetric_.LogHistBuilder.AddBucketValue(value);
                break;

            default:
                return false;
        }
        return true;
    }

    bool OnString(const std::string_view& value) override {
        switch (State_.Current()) {
            case TState::COMMON_LABELS:
                PARSE_ENSURE(!LastLabelName_.empty(), "empty label name in common labels");
                MetricConsumer_->OnLabel(LastLabelName_, std::string{value});
                break;

            case TState::METRIC_LABELS:
                PARSE_ENSURE(!LastLabelName_.empty(), "empty label name in metric labels");
                LastMetric_.Labels.Add(LastLabelName_, std::string{value});
                break;

            case TState::METRIC_NAME:
                PARSE_ENSURE(!value.empty(), "empty metric name");
                LastMetric_.Labels.Add(MetricNameLabel_, std::string{value});
                State_.ToPrev();
                break;

            case TState::COMMON_TS:
                MetricConsumer_->OnCommonTime(TInstant::ParseIso8601(value));
                State_.ToPrev();

                if (MetricConsumer_->NeedToStop()) {
                    IsIntentionallyHalted_ = true;
                    return false;
                }

                break;

            case TState::METRIC_TS:
                LastMetric_.SetLastTime(TInstant::ParseIso8601(value));
                State_.ToPrev();
                break;

            case TState::METRIC_VALUE:
                if (auto [doubleValue, ok] = ParseSpecDouble(value); ok) {
                    LastMetric_.SetLastValue(doubleValue);
                } else {
                    return false;
                }
                State_.ToPrev();
                break;

            case TState::METRIC_TYPE:
                LastMetric_.Type = MetricTypeFromStr(value);
                State_.ToPrev();
                break;

            case TState::METRIC_MODE:
                if (value == std::string_view("deriv")) {
                    LastMetric_.Type = EMetricType::RATE;
                }
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_SUM:
                if (auto [doubleValue, ok] = ParseSpecDouble(value); ok) {
                    LastMetric_.SummaryBuilder.SetSum(doubleValue);
                } else {
                    return false;
                }
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_MIN:
                if (auto [doubleValue, ok] = ParseSpecDouble(value); ok) {
                    LastMetric_.SummaryBuilder.SetMin(doubleValue);
                } else {
                    return false;
                }
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_MAX:
                if (auto [doubleValue, ok] = ParseSpecDouble(value); ok) {
                    LastMetric_.SummaryBuilder.SetMax(doubleValue);
                } else {
                    return false;
                }
                State_.ToPrev();
                break;

            case TState::METRIC_DSUMMARY_LAST:
                if (auto [doubleValue, ok] = ParseSpecDouble(value); ok) {
                    LastMetric_.SummaryBuilder.SetLast(doubleValue);
                } else {
                    return false;
                }
                State_.ToPrev();
                break;

            default:
                return false;
        }

        return true;
    }

    bool OnMapKey(const std::string_view& key) override {
        switch (State_.Current()) {
            case TState::ROOT_OBJECT:
                if (key == std::string_view("commonLabels") || key == std::string_view("labels")) {
                    State_.ToNext(TState::COMMON_LABELS);
                } else if (key == std::string_view("ts")) {
                    State_.ToNext(TState::COMMON_TS);
                } else if (key == std::string_view("sensors") || key == std::string_view("metrics")) {
                    State_.ToNext(TState::METRICS_ARRAY);
                }
                break;

            case TState::COMMON_LABELS:
            case TState::METRIC_LABELS:
                LastLabelName_ = key;
                break;

            case TState::METRIC_OBJECT:
                if (key == std::string_view("labels")) {
                    State_.ToNext(TState::METRIC_LABELS);
                } else if (key == std::string_view("name")) {
                    State_.ToNext(TState::METRIC_NAME);
                } else if (key == std::string_view("ts")) {
                    PARSE_ENSURE(!LastMetric_.SeenTimeseries,
                             "mixed timeseries and ts attributes");
                    LastMetric_.SeenTsOrValue = true;
                    State_.ToNext(TState::METRIC_TS);
                } else if (key == std::string_view("value")) {
                    PARSE_ENSURE(!LastMetric_.SeenTimeseries,
                             "mixed timeseries and value attributes");
                    LastMetric_.SeenTsOrValue = true;
                    State_.ToNext(TState::METRIC_VALUE);
                } else if (key == std::string_view("timeseries")) {
                    PARSE_ENSURE(!LastMetric_.SeenTsOrValue,
                             "mixed timeseries and ts/value attributes");
                    LastMetric_.SeenTimeseries = true;
                    State_.ToNext(TState::METRIC_TIMESERIES);
                } else if (key == std::string_view("mode")) {
                    State_.ToNext(TState::METRIC_MODE);
                } else if (key == std::string_view("kind") || key == std::string_view("type")) {
                    State_.ToNext(TState::METRIC_TYPE);
                } else if (key == std::string_view("hist")) {
                    State_.ToNext(TState::METRIC_HIST);
                } else if (key == std::string_view("summary")) {
                    State_.ToNext(TState::METRIC_DSUMMARY);
                } else if (key == std::string_view("log_hist")) {
                    State_.ToNext(TState::METRIC_LOG_HIST);
                } else if (key == std::string_view("memOnly")) {
                    // deprecated. Skip it without errors for backward compatibility
                } else {
                    ErrorMsg_ = NUtils::TYdbStringBuilder() << "unexpected key \"" << key << "\" in a metric schema";
                    return false;
                }
                break;

            case TState::METRIC_TIMESERIES:
                if (key == std::string_view("ts")) {
                    State_.ToNext(TState::METRIC_TS);
                } else if (key == std::string_view("value")) {
                    State_.ToNext(TState::METRIC_VALUE);
                } else if (key == std::string_view("hist")) {
                    State_.ToNext(TState::METRIC_HIST);
                } else if (key == std::string_view("summary")) {
                    State_.ToNext(TState::METRIC_DSUMMARY);
                } else if (key == std::string_view("log_hist")) {
                    State_.ToNext(TState::METRIC_LOG_HIST);
                }
                break;

            case TState::METRIC_HIST:
                if (key == std::string_view("bounds")) {
                    State_.ToNext(TState::METRIC_HIST_BOUNDS);
                } else if (key == std::string_view("buckets")) {
                    State_.ToNext(TState::METRIC_HIST_BUCKETS);
                } else if (key == std::string_view("inf")) {
                    State_.ToNext(TState::METRIC_HIST_INF);
                }
                break;

            case TState::METRIC_LOG_HIST:
                if (key == std::string_view("base")) {
                    State_.ToNext(TState::METRIC_LOG_HIST_BASE);
                } else if (key == std::string_view("zeros_count")) {
                    State_.ToNext(TState::METRIC_LOG_HIST_ZEROS);
                } else if (key == std::string_view("start_power")) {
                    State_.ToNext(TState::METRIC_LOG_HIST_START_POWER);
                } else if (key == std::string_view("buckets")) {
                    State_.ToNext(TState::METRIC_LOG_HIST_BUCKETS);
                }
                break;

            case TState::METRIC_DSUMMARY:
                if (key == std::string_view("sum")) {
                    State_.ToNext(TState::METRIC_DSUMMARY_SUM);
                } else if (key == std::string_view("min")) {
                    State_.ToNext(TState::METRIC_DSUMMARY_MIN);
                } else if (key == std::string_view("max")) {
                    State_.ToNext(TState::METRIC_DSUMMARY_MAX);
                } else if (key == std::string_view("last")) {
                    State_.ToNext(TState::METRIC_DSUMMARY_LAST);
                } else if (key == std::string_view("count")) {
                    State_.ToNext(TState::METRIC_DSUMMARY_COUNT);
                }

                break;


            default:
                return false;
        }

        return true;
    }

    bool OnOpenMap() override {
        switch (State_.Current()) {
            case TState::ROOT_OBJECT:
                MetricConsumer_->OnStreamBegin();
                break;

            case TState::COMMON_LABELS:
                MetricConsumer_->OnLabelsBegin();
                break;

            case TState::METRICS_ARRAY:
                State_.ToNext(TState::METRIC_OBJECT);
                LastMetric_.Clear();
                break;

            default:
                break;
        }
        return true;
    }

    bool OnCloseMap() override {
        switch (State_.Current()) {
            case TState::ROOT_OBJECT:
                MetricConsumer_->OnStreamEnd();
                break;

            case TState::METRIC_LABELS:
                State_.ToPrev();
                break;

            case TState::COMMON_LABELS:
                MetricConsumer_->OnLabelsEnd();
                State_.ToPrev();

                if (MetricConsumer_->NeedToStop()) {
                    IsIntentionallyHalted_ = true;
                    return false;
                }

                break;

            case TState::METRIC_OBJECT:
                ConsumeMetric();
                State_.ToPrev();
                break;

            case TState::METRIC_TIMESERIES:
                LastMetric_.SaveLastPoint();
                break;

            case TState::METRIC_HIST:
            case TState::METRIC_DSUMMARY:
            case TState::METRIC_LOG_HIST:
                State_.ToPrev();
                break;

            default:
                break;
        }
        return true;
    }

    bool OnOpenArray() override {
        auto currentState = State_.Current();
        PARSE_ENSURE(
            currentState == TState::METRICS_ARRAY ||
            currentState == TState::METRIC_TIMESERIES ||
            currentState == TState::METRIC_HIST_BOUNDS ||
            currentState == TState::METRIC_HIST_BUCKETS ||
            currentState == TState::METRIC_LOG_HIST_BUCKETS,
            "unexpected array begin");
        return true;
    }

    bool OnCloseArray() override {
        switch (State_.Current()) {
            case TState::METRICS_ARRAY:
            case TState::METRIC_TIMESERIES:
            case TState::METRIC_HIST_BOUNDS:
            case TState::METRIC_HIST_BUCKETS:
            case TState::METRIC_LOG_HIST_BUCKETS:
                State_.ToPrev();
                break;

            default:
                return false;
        }
        return true;
    }

    void OnError(size_t off, std::string_view reason) override {
        if (IsIntentionallyHalted_) {
            return;
        }

        size_t snippetBeg = (off < 20) ? 0 : (off - 20);
        std::string_view snippet = Data_.substr(snippetBeg, 40);

        throw TJsonDecodeError()
            << "cannot parse JSON, error at: " << off
            << ", reason: " << (ErrorMsg_.empty() ? reason : std::string_view{ErrorMsg_})
            << "\nsnippet: ..." << snippet << "...";
    }

    bool OnEnd() override {
        return true;
    }

    void ConsumeMetric() {
        // for backwad compatibility all unknown metrics treated as gauges
        if (LastMetric_.Type == EMetricType::UNKNOWN) {
            if (LastMetric_.HistogramBuilder.Empty()) {
                LastMetric_.Type = EMetricType::GAUGE;
            } else {
                LastMetric_.Type = EMetricType::HIST;
            }
        }

        // (1) begin metric
        MetricConsumer_->OnMetricBegin(LastMetric_.Type);

        // (2) labels
        if (!LastMetric_.Labels.empty()) {
            MetricConsumer_->OnLabelsBegin();
            for (auto&& label : LastMetric_.Labels) {
                MetricConsumer_->OnLabel(label.Name(), label.Value());
            }
            MetricConsumer_->OnLabelsEnd();
        }

        // (3) values
        switch (LastMetric_.Type) {
            case EMetricType::GAUGE:
                LastMetric_.Consume([this](TInstant time, EMetricValueType valueType, TMetricValue value) {
                    MetricConsumer_->OnDouble(time, value.AsDouble(valueType));
                });
                break;

            case EMetricType::IGAUGE:
                LastMetric_.Consume([this](TInstant time, EMetricValueType valueType, TMetricValue value) {
                    MetricConsumer_->OnInt64(time, value.AsInt64(valueType));
                });
                break;

            case EMetricType::COUNTER:
            case EMetricType::RATE:
                LastMetric_.Consume([this](TInstant time, EMetricValueType valueType, TMetricValue value) {
                    MetricConsumer_->OnUint64(time, value.AsUint64(valueType));
                });
                break;

            case EMetricType::HIST:
            case EMetricType::HIST_RATE:
                if (LastMetric_.TimeSeries.empty()) {
                    auto time = LastMetric_.LastPoint.GetTime();
                    auto histogram = LastMetric_.HistogramBuilder.Build();
                    MetricConsumer_->OnHistogram(time, histogram);
                } else {
                    for (const auto& p : LastMetric_.TimeSeries) {
                        DECODE_ENSURE(p.GetValueType() == EMetricValueType::HISTOGRAM, "Value is not a histogram");
                        MetricConsumer_->OnHistogram(p.GetTime(), p.GetValue().AsHistogram());
                    }
                }
                break;

            case EMetricType::DSUMMARY:
                if (LastMetric_.TimeSeries.empty()) {
                    auto time = LastMetric_.LastPoint.GetTime();
                    auto summary = LastMetric_.SummaryBuilder.Build();
                    MetricConsumer_->OnSummaryDouble(time, summary);
                } else {
                    for (const auto& p : LastMetric_.TimeSeries) {
                        DECODE_ENSURE(p.GetValueType() == EMetricValueType::SUMMARY, "Value is not a summary");
                        MetricConsumer_->OnSummaryDouble(p.GetTime(), p.GetValue().AsSummaryDouble());
                    }
                }
                break;

            case EMetricType::LOGHIST:
                if (LastMetric_.TimeSeries.empty()) {
                    auto time = LastMetric_.LastPoint.GetTime();
                    auto logHist = LastMetric_.LogHistBuilder.Build();
                    MetricConsumer_->OnLogHistogram(time, logHist);
                } else {
                    for (const auto& p : LastMetric_.TimeSeries) {
                        DECODE_ENSURE(p.GetValueType() == EMetricValueType::LOGHISTOGRAM, "Value is not a log_histogram");
                        MetricConsumer_->OnLogHistogram(p.GetTime(), p.GetValue().AsLogHistogram());
                    }
                }
                break;

            case EMetricType::UNKNOWN:
                // TODO: output metric labels
                ythrow yexception() << "unknown metric type";
        }

        // (4) end metric
        MetricConsumer_->OnMetricEnd();
    }

private:
    std::string_view Data_;
    IHaltableMetricConsumer* MetricConsumer_;
    std::string MetricNameLabel_;
    TState State_;
    std::string LastLabelName_;
    TMetricCollector LastMetric_;
    std::string ErrorMsg_;
    bool IsIntentionallyHalted_{false};
};

} // namespace

void DecodeJson(std::string_view data, IMetricConsumer* c, std::string_view metricNameLabel) {
    TCommonPartsCollector commonPartsCollector;
    {
        TMemoryInput memIn(data);
        TDecoderJson decoder(data, &commonPartsCollector, metricNameLabel);
        // no need to check a return value. If there is an error, a TJsonDecodeError is thrown
        NJson::ReadJson(&memIn, &decoder);
    }

    TCommonPartsProxy commonPartsProxy(std::move(commonPartsCollector.CommonParts()), c);
    {
        TMemoryInput memIn(data);
        TDecoderJson decoder(data, &commonPartsProxy, metricNameLabel);
        // no need to check a return value. If there is an error, a TJsonDecodeError is thrown
        NJson::ReadJson(&memIn, &decoder);
    }
}

#undef DECODE_ENSURE

}
