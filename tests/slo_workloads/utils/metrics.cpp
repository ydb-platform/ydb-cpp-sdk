#include "metrics.h"
#include "utils.h"

#include <util/system/env.h>

#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/meter_context.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/resource/resource.h>

#include <hdr/hdr_histogram.h>

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

using namespace std::chrono_literals;

namespace {

constexpr std::int64_t kHdrMinLatencyNs = 1'000;          // 1 us
constexpr std::int64_t kHdrMaxLatencyNs = 60'000'000'000; // 60 s
constexpr int kHdrSignificantFigures = 3;

// Thread-safe HDR histogram. Only successful latencies are recorded; errors
// are excluded from the percentile stream (operation_status="success").
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

    struct TPercentiles {
        double P50 = 0.0;
        double P95 = 0.0;
        double P99 = 0.0;
        bool HasData = false;
    };

    // Snapshot all three percentiles from one consistent HDR state and reset
    // the window — so each export cycle's gauge reflects only the last
    // interval's latencies. Reading p50/p95/p99 in one critical section
    // matches the Java workload's batch-callback pattern (avoids the race
    // where p99 would observe a histogram already reset by p50).
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

// Process-wide pusher: ONE MeterProvider with one OTLP exporter shared by
// all operation types. Publishing duplicate MeterProviders against the same
// Prometheus endpoint produces racing `target_info` writes for the same
// resource label set, which Prometheus rejects as `out of order sample`.
class TOtelSharedPusher {
public:
    TOtelSharedPusher(
        const std::string& metricsPushUrl,
        std::map<std::string, std::string> resourceAttributes,
        const std::string& meterSchemaVersion
    )
        : CommonAttributes_(std::move(resourceAttributes))
    {
        StubRetry_ = !GetEnv("SLO_STUB_RETRY").empty();

        auto exporterOptions = opentelemetry::exporter::otlp::OtlpHttpMetricExporterOptions();
        exporterOptions.url = metricsPushUrl;

        auto exporter = opentelemetry::exporter::otlp::OtlpHttpMetricExporterFactory::Create(exporterOptions);

        opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions readerOptions;
        readerOptions.export_interval_millis = 1000ms;
        readerOptions.export_timeout_millis = 500ms;

        auto metricReader = opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(
            std::move(exporter), readerOptions);

        auto context = std::make_unique<opentelemetry::sdk::metrics::MeterContext>(
            std::unique_ptr<opentelemetry::sdk::metrics::ViewRegistry>(new opentelemetry::sdk::metrics::ViewRegistry()),
            opentelemetry::sdk::resource::Resource::Create(
                opentelemetry::common::MakeKeyValueIterableView(CommonAttributes_))
        );

        MeterProvider_ = opentelemetry::sdk::metrics::MeterProviderFactory::Create(std::move(context));
        MeterProvider_->AddMetricReader(std::move(metricReader));

        Meter_ = MeterProvider_->GetMeter("slo_workloads", meterSchemaVersion);

        InitMetrics();
    }

    ~TOtelSharedPusher() {
        // Remove observable-gauge callbacks before MeterProvider tears down
        // the readers, so a final collection in flight cannot see this object
        // half-destroyed.
        if (LatencyP50_) LatencyP50_->RemoveCallback(&TOtelSharedPusher::ObserveP50, this);
        if (LatencyP95_) LatencyP95_->RemoveCallback(&TOtelSharedPusher::ObserveP95, this);
        if (LatencyP99_) LatencyP99_->RemoveCallback(&TOtelSharedPusher::ObserveP99, this);
        // MeterProvider destructor calls Shutdown(); do not call it explicitly
        // here — the OTel SDK rejects a second Shutdown with a warning.
    }

    void Record(const std::string& operationType, const TRequestData& data) {
        const bool success = data.Status == NYdb::EStatus::SUCCESS;
        auto& series = GetOrCreateSeries(operationType);

        OperationsTotal_->Add(uint64_t{1},
            opentelemetry::common::MakeKeyValueIterableView(
                success ? series.SuccessAttrs : series.ErrorAttrs));

        // userver TableClient does not expose per-request retry count.
        if (!StubRetry_) {
            RetryAttemptsTotal_->Add(data.RetryAttempts + 1,
                opentelemetry::common::MakeKeyValueIterableView(series.RetryAttrs));
        }

        if (success) {
            series.Recorder.Record(data.Delay);
        }
    }

private:
    struct TSeries {
        TLatencyRecorder Recorder;
        // Pre-merged attribute maps used on the hot path so Record() does not
        // allocate / copy CommonAttributes_ per call.
        std::map<std::string, std::string> SuccessAttrs;
        std::map<std::string, std::string> ErrorAttrs;
        std::map<std::string, std::string> RetryAttrs;

        // Cached snapshot, refreshed by EnsureSnapshot() in whichever
        // observable-gauge callback fires first per export cycle. All three
        // callbacks read from these atomics so p50/p95/p99 land in the same
        // export with consistent values from one HDR snapshot — independent
        // of the order the SDK iterates instruments.
        std::atomic<double> P50{0.0};
        std::atomic<double> P95{0.0};
        std::atomic<double> P99{0.0};
        std::atomic<bool> HasData{false};
        std::mutex SnapshotMutex;
        std::chrono::steady_clock::time_point LastSnapshot{};
    };

    // Half the export interval — guarantees one snapshot per cycle while
    // tolerating arbitrary callback ordering.
    static constexpr std::chrono::milliseconds kSnapshotFreshness{500};

    void EnsureSnapshot(TSeries& series) {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard lock(series.SnapshotMutex);
        if (now - series.LastSnapshot < kSnapshotFreshness) {
            return;
        }
        auto snap = series.Recorder.SnapshotAndReset();
        if (snap.HasData) {
            series.P50.store(snap.P50);
            series.P95.store(snap.P95);
            series.P99.store(snap.P99);
            series.HasData.store(true);
        } else {
            series.HasData.store(false);
        }
        series.LastSnapshot = now;
    }

    TSeries& GetOrCreateSeries(const std::string& op) {
        {
            std::shared_lock lock(SeriesMutex_);
            auto it = Series_.find(op);
            if (it != Series_.end()) {
                return *it->second;
            }
        }
        std::unique_lock lock(SeriesMutex_);
        auto& slot = Series_[op];
        if (!slot) {
            slot = std::make_unique<TSeries>();
            slot->SuccessAttrs = CommonAttributes_;
            slot->SuccessAttrs["operation_type"] = op;
            slot->SuccessAttrs["operation_status"] = "success";
            slot->ErrorAttrs = CommonAttributes_;
            slot->ErrorAttrs["operation_type"] = op;
            slot->ErrorAttrs["operation_status"] = "error";
            slot->RetryAttrs = CommonAttributes_;
            slot->RetryAttrs["operation_type"] = op;
        }
        return *slot;
    }

    void InitMetrics() {
        OperationsTotal_ = Meter_->CreateUInt64Counter("sdk_operations_total",
            "Total number of operations, categorized by operation type and status.");
        RetryAttemptsTotal_ = Meter_->CreateUInt64Counter("sdk_retry_attempts_total",
            "Total number of retry attempts (including the first attempt), categorized by operation type.");

        LatencyP50_ = Meter_->CreateDoubleObservableGauge(
            "sdk_operation_latency_p50_seconds",
            "P50 latency of successful operations in seconds.", "s");
        LatencyP95_ = Meter_->CreateDoubleObservableGauge(
            "sdk_operation_latency_p95_seconds",
            "P95 latency of successful operations in seconds.", "s");
        LatencyP99_ = Meter_->CreateDoubleObservableGauge(
            "sdk_operation_latency_p99_seconds",
            "P99 latency of successful operations in seconds.", "s");

        LatencyP50_->AddCallback(&TOtelSharedPusher::ObserveP50, this);
        LatencyP95_->AddCallback(&TOtelSharedPusher::ObserveP95, this);
        LatencyP99_->AddCallback(&TOtelSharedPusher::ObserveP99, this);
    }

    static void ObserveP50(opentelemetry::metrics::ObserverResult r, void* s) {
        ObservePercentile(r, s, &TSeries::P50);
    }
    static void ObserveP95(opentelemetry::metrics::ObserverResult r, void* s) {
        ObservePercentile(r, s, &TSeries::P95);
    }
    static void ObserveP99(opentelemetry::metrics::ObserverResult r, void* s) {
        ObservePercentile(r, s, &TSeries::P99);
    }

    // Each callback ensures a fresh snapshot exists for the current export
    // cycle (EnsureSnapshot is a no-op if one was taken less than
    // kSnapshotFreshness ago). Whichever of P50/P95/P99 the SDK invokes first
    // performs the snapshot+reset; the others read cached atomics. Order
    // between the three callbacks is irrelevant.
    static void ObservePercentile(opentelemetry::metrics::ObserverResult result, void* state,
                                  std::atomic<double> TSeries::*field)
    {
        auto* self = static_cast<TOtelSharedPusher*>(state);
        auto obs = opentelemetry::nostd::get<
            opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<double>>>(result);

        std::shared_lock lock(self->SeriesMutex_);
        for (const auto& [op, series] : self->Series_) {
            self->EnsureSnapshot(*series);
            if (!series->HasData.load()) {
                continue;
            }
            obs->Observe((series.get()->*field).load(),
                opentelemetry::common::MakeKeyValueIterableView(series->SuccessAttrs));
        }
    }

    std::map<std::string, std::string> CommonAttributes_;
    bool StubRetry_ = false;

    std::unique_ptr<opentelemetry::sdk::metrics::MeterProvider> MeterProvider_;
    std::shared_ptr<opentelemetry::metrics::Meter> Meter_;

    std::unique_ptr<opentelemetry::metrics::Counter<uint64_t>> OperationsTotal_;
    std::unique_ptr<opentelemetry::metrics::Counter<uint64_t>> RetryAttemptsTotal_;
    std::shared_ptr<opentelemetry::metrics::ObservableInstrument> LatencyP50_;
    std::shared_ptr<opentelemetry::metrics::ObservableInstrument> LatencyP95_;
    std::shared_ptr<opentelemetry::metrics::ObservableInstrument> LatencyP99_;

    std::shared_mutex SeriesMutex_;
    std::unordered_map<std::string, std::unique_ptr<TSeries>> Series_;
};

struct TSharedPusherKey {
    std::string Url;
    std::map<std::string, std::string> ResourceAttributes;
    std::string MeterSchemaVersion;

    bool operator==(const TSharedPusherKey& other) const {
        return Url == other.Url
            && ResourceAttributes == other.ResourceAttributes
            && MeterSchemaVersion == other.MeterSchemaVersion;
    }
};

struct TSharedPusherKeyHash {
    size_t operator()(const TSharedPusherKey& key) const {
        size_t h = std::hash<std::string>{}(key.Url);
        for (const auto& [k, v] : key.ResourceAttributes) {
            h ^= std::hash<std::string>{}(k) ^ std::hash<std::string>{}(v);
        }
        h ^= std::hash<std::string>{}(key.MeterSchemaVersion);
        return h;
    }
};

std::mutex g_sharedMu;
std::unordered_map<TSharedPusherKey, std::weak_ptr<TOtelSharedPusher>, TSharedPusherKeyHash> g_shared;

std::shared_ptr<TOtelSharedPusher> GetOrCreateSharedPusher(
    const std::string& url,
    const std::map<std::string, std::string>& resourceAttributes,
    const std::string& meterSchemaVersion
) {
    TSharedPusherKey key{url, resourceAttributes, meterSchemaVersion};
    std::lock_guard lock(g_sharedMu);
    auto it = g_shared.find(key);
    if (it != g_shared.end()) {
        if (auto sp = it->second.lock()) {
            return sp;
        }
    }
    auto sp = std::make_shared<TOtelSharedPusher>(url, resourceAttributes, meterSchemaVersion);
    g_shared[key] = sp;
    return sp;
}

class TOtelMetricsPusherWrapper : public IMetricsPusher {
public:
    TOtelMetricsPusherWrapper(std::shared_ptr<TOtelSharedPusher> shared, std::string operationType)
        : Shared_(std::move(shared))
        , OperationType_(std::move(operationType))
    {}

    void PushRequestData(const TRequestData& requestData) override {
        Shared_->Record(OperationType_, requestData);
    }

private:
    std::shared_ptr<TOtelSharedPusher> Shared_;
    std::string OperationType_;
};

class TNoopMetricsPusher : public IMetricsPusher {
public:
    void PushRequestData([[maybe_unused]] const TRequestData& requestData) override {}
};

} // namespace

std::unique_ptr<IMetricsPusher> CreateOtelMetricsPusher(
    const std::string& metricsPushUrl,
    const std::string& operationType)
{
    return std::make_unique<TOtelMetricsPusherWrapper>(
        GetOrCreateSharedPusher(
            metricsPushUrl,
            MakeNativeSloOtelResourceAttributes(),
            NativeSloMeterSchemaVersion()),
        operationType);
}

std::unique_ptr<IMetricsPusher> CreateNoopMetricsPusher() {
    return std::make_unique<TNoopMetricsPusher>();
}
