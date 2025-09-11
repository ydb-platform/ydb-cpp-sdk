#include "provider.h"

#include <library/cpp/monlib/metrics/histogram_collector.h>
#include <library/cpp/monlib/metrics/metric_registry.h>


namespace NYdb::inline V3::NMetrics {

class TMonlibIntGauge: public NMetrics::IIntGauge {
public:
    TMonlibIntGauge(::NMonitoring::IIntGauge* gauge)
        : Gauge_(gauge)
    {}

    void Add(std::int64_t value) override {
        Gauge_->Add(value);
    }

    void Set(std::int64_t value) override {
        Gauge_->Set(value);
    }

private:
    ::NMonitoring::IIntGauge* Gauge_;
};

class TMonlibCounter: public NMetrics::ICounter {
public:
    TMonlibCounter(::NMonitoring::ICounter* counter)
        : Counter_(counter)
    {}

    void Add(std::uint64_t value) override {
        Counter_->Add(value);
    }

private:
    ::NMonitoring::ICounter* Counter_;
};

class TMonlibRate: public NMetrics::IRate {
public:
    TMonlibRate(::NMonitoring::IRate* rate)
        : Rate_(rate)
    {}

    void Add(std::uint64_t value) override {
        Rate_->Add(value);
    }

private:
    ::NMonitoring::IRate* Rate_;
};

class TMonlibHistogram: public NMetrics::IHistogram {
public:
    TMonlibHistogram(::NMonitoring::IHistogram* histogram)
        : Histogram_(histogram)
    {}

    void Record(double value) override {
        Histogram_->Record(value);
    }

private:
    ::NMonitoring::IHistogram* Histogram_;
};

class TMonlibMetricsProvider: public NMetrics::IMetricsProvider {
public:
    TMonlibMetricsProvider(NMonitoring::IMetricRegistry* registry)
        : Registry_(registry)
    {}

    ::NMonitoring::IMetricRegistry* GetRegistry() const {
        return Registry_;
    }

    std::shared_ptr<IIntGauge> IntGauge(TLabels labels) override {
        return std::make_shared<TMonlibIntGauge>(Registry_->IntGauge(MakeLabels(labels)));
    }

    std::shared_ptr<ICounter> Counter(TLabels labels) override {
        return std::make_shared<TMonlibCounter>(Registry_->Counter(MakeLabels(labels)));
    }

    std::shared_ptr<IRate> Rate(TLabels labels) override {
        return std::make_shared<TMonlibRate>(Registry_->Rate(MakeLabels(labels)));
    }

    std::shared_ptr<IHistogram> HistogramCounter(TLabels labels, TBucketBounds bounds) override {
        return std::make_shared<TMonlibHistogram>(
            Registry_->HistogramCounter(MakeLabels(labels), MakeHistogramCollector(bounds)));
    }

    std::shared_ptr<IHistogram> HistogramRate(TLabels labels, TBucketBounds bounds) override {
        return std::make_shared<TMonlibHistogram>(
            Registry_->HistogramRate(MakeLabels(labels), MakeHistogramCollector(bounds)));
    }

private:
    ::NMonitoring::ILabelsPtr MakeLabels(TLabels labels) {
        auto monlibLabels = MakeIntrusive<::NMonitoring::TLabels>();
        for (const auto& label : labels) {
            monlibLabels->Add(TStringBuf(label.Name), TStringBuf(label.Value));
        }
        return monlibLabels;
    }

    ::NMonitoring::IHistogramCollectorPtr MakeHistogramCollector(TBucketBounds bounds) {
        return ::NMonitoring::ExplicitHistogram(TVector<double>(bounds.begin(), bounds.end()));
    }

    NMonitoring::IMetricRegistry* Registry_;
};

std::shared_ptr<NMetrics::IMetricsProvider> CreateMonlibMetricsProvider(::NMonitoring::IMetricRegistry* registry) {
    return std::make_shared<TMonlibMetricsProvider>(registry);
}

::NMonitoring::IMetricRegistry* TryGetUnderlyingMetricsRegistry(std::shared_ptr<NMetrics::IMetricsProvider> metricsProvider) {
    if (auto monlibMetricsProvider = std::dynamic_pointer_cast<TMonlibMetricsProvider>(metricsProvider)) {
        return monlibMetricsProvider->GetRegistry();
    }
    return nullptr;
}

}
