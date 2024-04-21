#include "encode.h"

#include <ydb-cpp-sdk/library/monlib/encode/encoder.h>
#include <ydb-cpp-sdk/library/monlib/encode/json/json.h>
#include <src/library/monlib/encode/spack/spack_v1.h>
#include <src/library/monlib/encode/prometheus/prometheus.h>

#include <src/util/stream/str.h>

namespace NMonitoring {
    namespace {
        constexpr TInstant ZERO_TIME = TInstant::Zero();

        class TConsumer final: public ICountableConsumer {
            using TLabel = std::pair<std::string, std::string>; // name, value

        public:
            explicit TConsumer(NMonitoring::IMetricEncoderPtr encoderImpl, TCountableBase::EVisibility vis)
                : EncoderImpl_(std::move(encoderImpl))
                , Visibility_{vis}
            {
            }

            void OnCounter(
                const std::string& labelName, const std::string& labelValue,
                const TCounterForPtr* counter) override {
                NMonitoring::EMetricType metricType = counter->ForDerivative()
                                                          ? NMonitoring::EMetricType::RATE
                                                          : NMonitoring::EMetricType::GAUGE;
                EncoderImpl_->OnMetricBegin(metricType);
                EncodeLabels(labelName, labelValue);

                if (metricType == NMonitoring::EMetricType::GAUGE) {
                    EncoderImpl_->OnDouble(ZERO_TIME, static_cast<double>(counter->Val()));
                } else {
                    EncoderImpl_->OnUint64(ZERO_TIME, counter->Val());
                }

                EncoderImpl_->OnMetricEnd();
            }

            void OnHistogram(
                const std::string& labelName, const std::string& labelValue,
                IHistogramSnapshotPtr snapshot, bool derivative) override {
                NMonitoring::EMetricType metricType = derivative ? EMetricType::HIST_RATE : EMetricType::HIST;

                EncoderImpl_->OnMetricBegin(metricType);
                EncodeLabels(labelName, labelValue);
                EncoderImpl_->OnHistogram(ZERO_TIME, snapshot);
                EncoderImpl_->OnMetricEnd();
            }

            void OnGroupBegin(
                const std::string& labelName, const std::string& labelValue,
                const TDynamicCounters*) override {
                if (labelName.empty() && labelValue.empty()) {
                    // root group has empty label name and value
                    EncoderImpl_->OnStreamBegin();
                } else {
                    ParentLabels_.emplace_back(labelName, labelValue);
                }
            }

            void OnGroupEnd(
                const std::string& labelName, const std::string& labelValue,
                const TDynamicCounters*) override {
                if (labelName.empty() && labelValue.empty()) {
                    // root group has empty label name and value
                    EncoderImpl_->OnStreamEnd();
                    EncoderImpl_->Close();
                } else {
                    ParentLabels_.pop_back();
                }
            }

            TCountableBase::EVisibility Visibility() const override {
                return Visibility_;
            }

        private:
            void EncodeLabels(const std::string& labelName, const std::string& labelValue) {
                EncoderImpl_->OnLabelsBegin();
                for (const auto& label : ParentLabels_) {
                    EncoderImpl_->OnLabel(label.first, label.second);
                }
                EncoderImpl_->OnLabel(labelName, labelValue);
                EncoderImpl_->OnLabelsEnd();
            }

        private:
            NMonitoring::IMetricEncoderPtr EncoderImpl_;
            std::vector<TLabel> ParentLabels_;
            TCountableBase::EVisibility Visibility_;
        };

    }

    std::unique_ptr<ICountableConsumer> CreateEncoder(IOutputStream* out, EFormat format,
        std::string_view nameLabel, TCountableBase::EVisibility vis)
    {
        switch (format) {
            case EFormat::JSON:
                return MakeHolder<TConsumer>(NMonitoring::EncoderJson(out), vis);
            case EFormat::SPACK:
                return MakeHolder<TConsumer>(NMonitoring::EncoderSpackV1(
                    out,
                    NMonitoring::ETimePrecision::SECONDS,
                    NMonitoring::ECompression::ZSTD), vis);
            case EFormat::PROMETHEUS:
                return MakeHolder<TConsumer>(NMonitoring::EncoderPrometheus(
                    out, nameLabel), vis);
            default:
                ythrow yexception() << "unsupported metric encoding format: " << format;
                break;
        }
    }

    std::unique_ptr<ICountableConsumer> AsCountableConsumer(IMetricEncoderPtr encoder, TCountableBase::EVisibility visibility) {
        return MakeHolder<TConsumer>(std::move(encoder), visibility);
    }

    void ToJson(const TDynamicCounters& counters, IOutputStream* out) {
        TConsumer consumer{EncoderJson(out), TCountableBase::EVisibility::Public};
        counters.Accept(std::string{}, std::string{}, consumer);
    }

    std::string ToJson(const TDynamicCounters& counters) {
        TStringStream ss;
        ToJson(counters, &ss);
        return ss.Str();
    }

}
