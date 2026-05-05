#include "metrics.h"

#include "utils.h"

#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h>

#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/metrics/meter_context.h>
#include <opentelemetry/sdk/resource/resource.h>

#include <ydb-cpp-sdk/client/resources/ydb_resources.h>

#include <util/system/env.h>

#include <hdr/hdr_histogram.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>


using namespace std::chrono_literals;

namespace {

constexpr std::int64_t kHdrMinLatencyNs = 1'000;          // 1 us
constexpr std::int64_t kHdrMaxLatencyNs = 60'000'000'000; // 60 s
constexpr int kHdrSignificantFigures = 3;

std::string ResolveWorkloadRef() {
    std::string ref = GetEnv("WORKLOAD_REF");
    return ref.empty() ? "unknown" : ref;
}

// Minimal thread-safe wrapper around hdr_histogram for a single
// (operation_type, operation_status="success") series. Only successful
// latencies are recorded; errors are excluded from the percentile stream
// per deploy/metrics.yaml.
class TLatencyRecorder {
public:
    TLatencyRecorder() {
        hdr_histogram* raw = nullptr;
        int rc = hdr_init(kHdrMinLatencyNs, kHdrMaxLatencyNs, kHdrSignificantFigures, &raw);
        Y_ABORT_UNLESS(rc == 0, "hdr_init failed: %d", rc);
        Histogram_.reset(raw);
    }

    void Record(TDuration d) {
        std::int64_t ns = static_cast<std::int64_t>(d.NanoSeconds());
        if (ns < kHdrMinLatencyNs) {
            ns = kHdrMinLatencyNs;
        } else if (ns > kHdrMaxLatencyNs) {
            ns = kHdrMaxLatencyNs;
        }
        std::lock_guard lock(Mutex_);
        hdr_record_value(Histogram_.get(), ns);
    }

    // Returns p50/p95/p99 as seconds and resets the recorder window so
    // gauges reflect only the most recent interval.
    struct TPercentiles {
        double P50 = 0.0;
        double P95 = 0.0;
        double P99 = 0.0;
        bool HasData = false;
    };

    TPercentiles SnapshotAndReset() {
        TPercentiles out;
        std::lock_guard lock(Mutex_);
        if (Histogram_->total_count == 0) {
            return out;
        }
        out.HasData = true;
        out.P50 = hdr_value_at_percentile(Histogram_.get(), 50.0) / 1e9;
        out.P95 = hdr_value_at_percentile(Histogram_.get(), 95.0) / 1e9;
        out.P99 = hdr_value_at_percentile(Histogram_.get(), 99.0) / 1e9;
        hdr_reset(Histogram_.get());
        return out;
    }

private:
    struct THdrDeleter {
        void operator()(hdr_histogram* h) const noexcept { if (h) hdr_close(h); }
    };

    std::mutex Mutex_;
    std::unique_ptr<hdr_histogram, THdrDeleter> Histogram_;
};

class TOtelMetricsPusher : public IMetricsPusher {
public:
    TOtelMetricsPusher(const std::string& metricsPushUrl, const std::string& operationType)
        : OperationType_(operationType)
        , Ref_(ResolveWorkloadRef())
        , CommonAttributes_{
            {"ref", Ref_},
            {"sdk", "cpp"},
            {"sdk_version", NYdb::GetSdkSemver()}
        }
    {
        auto exporterOptions = opentelemetry::exporter::otlp::OtlpHttpMetricExporterOptions();
        exporterOptions.url = metricsPushUrl;

        auto exporter = opentelemetry::exporter::otlp::OtlpHttpMetricExporterFactory::Create(exporterOptions);

        opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions readerOptions;
        readerOptions.export_interval_millis = 1000ms;
        readerOptions.export_timeout_millis  = 900ms;

        auto metricReader = opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), readerOptions);

        auto context = std::make_unique<opentelemetry::sdk::metrics::MeterContext>(
            std::unique_ptr<opentelemetry::sdk::metrics::ViewRegistry>(new opentelemetry::sdk::metrics::ViewRegistry()),
            opentelemetry::sdk::resource::Resource::Create(opentelemetry::common::MakeKeyValueIterableView(CommonAttributes_))
        );

        MeterProvider_ = opentelemetry::sdk::metrics::MeterProviderFactory::Create(std::move(context));
        MeterProvider_->AddMetricReader(std::move(metricReader));

        Meter_ = MeterProvider_->GetMeter("slo_workloads", NYdb::GetSdkSemver());

        InitMetrics();
        StartPercentilePublisher();
    }

    ~TOtelMetricsPusher() override {
        PublisherShouldStop_.store(true);
        if (PublisherThread_.joinable()) {
            PublisherThread_.join();
        }
    }

    void PushRequestData(const TRequestData& requestData) override {
        const bool success = requestData.Status == NYdb::EStatus::SUCCESS;
        const std::string status = success ? "success" : "error";

        OperationsTotal_->Add(1, MergeAttributes({
            {"operation_type", OperationType_},
            {"operation_status", status},
        }));

        // sdk_retry_attempts_total = total number of technical attempts
        // including the first one. TStatUnit counts only post-first attempts,
        // so add 1 to include the initial attempt.
        RetryAttemptsTotal_->Add(static_cast<double>(requestData.RetryAttempts + 1),
            MergeAttributes({
                {"operation_type", OperationType_},
            })
        );

        if (success) {
            Latency_.Record(requestData.Delay);
        }
    }

private:
    void InitMetrics() {
        OperationsTotal_ = Meter_->CreateDoubleCounter("sdk_operations_total",
            "Total number of operations, categorized by operation type and status."
        );

        RetryAttemptsTotal_ = Meter_->CreateDoubleCounter("sdk_retry_attempts_total",
            "Total number of retry attempts (including the first attempt), categorized by operation type."
        );

        LatencyP50_ = Meter_->CreateDoubleGauge("sdk_operation_latency_p50_seconds",
            "P50 latency of successful operations in seconds.", "s"
        );
        LatencyP95_ = Meter_->CreateDoubleGauge("sdk_operation_latency_p95_seconds",
            "P95 latency of successful operations in seconds.", "s"
        );
        LatencyP99_ = Meter_->CreateDoubleGauge("sdk_operation_latency_p99_seconds",
            "P99 latency of successful operations in seconds.", "s"
        );
    }

    void StartPercentilePublisher() {
        PublisherThread_ = std::thread([this]() {
            while (!PublisherShouldStop_.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(1s);
                PublishPercentiles();
            }
            // Final flush before exit.
            PublishPercentiles();
        });
    }

    void PublishPercentiles() {
        auto snapshot = Latency_.SnapshotAndReset();
        if (!snapshot.HasData) {
            return;
        }
        auto attrs = MergeAttributes({
            {"operation_type", OperationType_},
            {"operation_status", "success"},
        });
        LatencyP50_->Record(snapshot.P50, attrs);
        LatencyP95_->Record(snapshot.P95, attrs);
        LatencyP99_->Record(snapshot.P99, attrs);
    }

    std::map<std::string, std::string> MergeAttributes(const std::map<std::string, std::string>& metricAttrs) const {
        std::map<std::string, std::string> result = CommonAttributes_;
        result.insert(metricAttrs.begin(), metricAttrs.end());
        return result;
    }

    std::string OperationType_;
    std::string Ref_;
    std::map<std::string, std::string> CommonAttributes_;

    std::unique_ptr<opentelemetry::sdk::metrics::MeterProvider> MeterProvider_;
    std::shared_ptr<opentelemetry::metrics::Meter> Meter_;

    std::unique_ptr<opentelemetry::metrics::Counter<double>> OperationsTotal_;
    std::unique_ptr<opentelemetry::metrics::Counter<double>> RetryAttemptsTotal_;
    std::unique_ptr<opentelemetry::metrics::Gauge<double>> LatencyP50_;
    std::unique_ptr<opentelemetry::metrics::Gauge<double>> LatencyP95_;
    std::unique_ptr<opentelemetry::metrics::Gauge<double>> LatencyP99_;

    TLatencyRecorder Latency_;
    std::thread PublisherThread_;
    std::atomic<bool> PublisherShouldStop_{false};
};

class TNoopMetricsPusher : public IMetricsPusher {
public:
    void PushRequestData([[maybe_unused]] const TRequestData& requestData) override {}
};

} // namespace

std::unique_ptr<IMetricsPusher> CreateOtelMetricsPusher(const std::string& metricsPushUrl, const std::string& operationType) {
    return std::make_unique<TOtelMetricsPusher>(metricsPushUrl, operationType);
}

std::unique_ptr<IMetricsPusher> CreateNoopMetricsPusher() {
    return std::make_unique<TNoopMetricsPusher>();
}
