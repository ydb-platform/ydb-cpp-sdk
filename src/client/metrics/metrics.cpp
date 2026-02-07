#include <ydb-cpp-sdk/client/metrics/metrics.h>

namespace NYdb::inline V3::NMetrics {

class TMetricsApiImpl : public IMetricsApi {
public:
    void SetMetricRegistry(std::shared_ptr<IMetricRegistry> registry) override {
        Registry_ = std::move(registry);
    }

    void SetTraceProvider(std::shared_ptr<ITraceProvider> provider) override {
        TraceProvider_ = std::move(provider);
    }

    std::shared_ptr<IMetricRegistry> GetMetricRegistry() const override {
        return Registry_;
    }

    std::shared_ptr<ITraceProvider> GetTraceProvider() const override {
        return TraceProvider_;
    }

private:
    std::shared_ptr<IMetricRegistry> Registry_;
    std::shared_ptr<ITraceProvider> TraceProvider_;
};

IMetricsApi* IMetricsApi::Create(TDriver driver) {
    return new TMetricsApiImpl();
}

} // namespace NYdb::NMetrics
