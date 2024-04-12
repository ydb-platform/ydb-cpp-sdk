#pragma once

#include "string_pool.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/encode/encoder.h>
#include <src/library/monlib/encode/encoder_state.h>
#include <ydb-cpp-sdk/library/monlib/encode/format.h>
#include <src/library/monlib/metrics/metric_value.h>

#include <ydb-cpp-sdk/util/datetime/base.h>
#include <ydb-cpp-sdk/util/digest/numeric.h>
=======
#include <src/library/monlib/encode/encoder.h>
#include <src/library/monlib/encode/encoder_state.h>
#include <src/library/monlib/encode/format.h>
#include <src/library/monlib/metrics/metric_value.h>

#include <src/util/datetime/base.h>
#include <src/util/digest/numeric.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))


namespace NMonitoring {

class TBufferedEncoderBase : public IMetricEncoder {
public:
    void OnStreamBegin() override;
    void OnStreamEnd() override;

    void OnCommonTime(TInstant time) override;

    void OnMetricBegin(EMetricType type) override;
    void OnMetricEnd() override;

    void OnLabelsBegin() override;
    void OnLabelsEnd() override;
    void OnLabel(std::string_view name, std::string_view value) override;
    void OnLabel(ui32 name, ui32 value) override;
    std::pair<ui32, ui32> PrepareLabel(std::string_view name, std::string_view value) override;

    void OnDouble(TInstant time, double value) override;
    void OnInt64(TInstant time, i64 value) override;
    void OnUint64(TInstant time, ui64 value) override;

    void OnHistogram(TInstant time, IHistogramSnapshotPtr snapshot) override;
    void OnSummaryDouble(TInstant time, ISummaryDoubleSnapshotPtr snapshot) override;
    void OnLogHistogram(TInstant, TLogHistogramSnapshotPtr) override;

protected:
    using TPooledStr = TStringPoolBuilder::TValue;

    struct TPooledLabel {
        TPooledLabel(const TPooledStr* key, const TPooledStr* value)
            : Key{key}
            , Value{value}
        {
        }

        bool operator==(const TPooledLabel& other) const {
            return std::tie(Key, Value) == std::tie(other.Key, other.Value);
        }

        bool operator!=(const TPooledLabel& other) const {
            return !(*this == other);
        }

        const TPooledStr* Key;
        const TPooledStr* Value;
    };

    using TPooledLabels = std::vector<TPooledLabel>;

    struct TPooledLabelsHash {
        size_t operator()(const TPooledLabels& val) const {
            size_t hash{0};

            for (auto v : val) {
                hash = CombineHashes<size_t>(hash, reinterpret_cast<size_t>(v.Key));
                hash = CombineHashes<size_t>(hash, reinterpret_cast<size_t>(v.Value));
            }

            return hash;
        }
    };

    using TMetricMap = THashMap<TPooledLabels, size_t, TPooledLabelsHash>;

    struct TMetric {
        EMetricType MetricType = EMetricType::UNKNOWN;
        TPooledLabels Labels;
        TMetricTimeSeries TimeSeries;
    };

protected:
    std::string FormatLabels(const TPooledLabels& labels) const;

protected:
    TEncoderState State_;

    TStringPoolBuilder LabelNamesPool_;
    TStringPoolBuilder LabelValuesPool_;
    TInstant CommonTime_ = TInstant::Zero();
    TPooledLabels CommonLabels_;
    std::vector<TMetric> Metrics_;
    TMetricMap MetricMap_;
    EMetricsMergingMode MetricsMergingMode_ = EMetricsMergingMode::DEFAULT;
};

}
