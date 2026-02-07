#pragma once

#include <ydb-cpp-sdk/client/extension_common/extension.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace NYdb::inline V3::NMetrics {

using TLabels = std::map<std::string, std::string>;

class ICounter {
public:
    virtual ~ICounter() = default;
    virtual void Inc() = 0;
};

class IGauge {
public:
    virtual ~IGauge() = default;
    virtual void Add(double delta) = 0;
    virtual void Set(double value) = 0;
};

class IHistogram {
public:
    virtual ~IHistogram() = default;
    virtual void Record(double value) = 0;
};

class IMetricRegistry {
public:
    virtual ~IMetricRegistry() = default;

    virtual std::shared_ptr<ICounter> Counter(const std::string& name, const TLabels& labels = {}) = 0;
    virtual std::shared_ptr<IGauge> Gauge(const std::string& name, const TLabels& labels = {}) = 0;
    virtual std::shared_ptr<IHistogram> Histogram(const std::string& name, const std::vector<double>& buckets, const TLabels& labels = {}) = 0;
};

enum class ESpanKind {
    INTERNAL,
    SERVER,
    CLIENT,
    PRODUCER,
    CONSUMER
};

class ISpan {
public:
    virtual ~ISpan() = default;
    virtual void End() = 0;
    virtual void SetAttribute(const std::string& key, const std::string& value) = 0;
    virtual void SetAttribute(const std::string& key, int64_t value) = 0;
    virtual void SetAttribute(const std::string& key, double value) = 0;
    virtual void SetAttribute(const std::string& key, bool value) = 0;
};

class ITracer {
public:
    virtual ~ITracer() = default;
    virtual std::shared_ptr<ISpan> StartSpan(const std::string& name, ESpanKind kind = ESpanKind::INTERNAL) = 0;
};

class ITraceProvider {
public:
    virtual ~ITraceProvider() = default;
    virtual std::shared_ptr<ITracer> GetTracer(const std::string& name) = 0;
};

class IMetricsApi : public IExtensionApi {
public:
    static IMetricsApi* Create(TDriver driver);
public:
    virtual ~IMetricsApi() = default;
    virtual void SetMetricRegistry(std::shared_ptr<IMetricRegistry> registry) = 0;
    virtual void SetTraceProvider(std::shared_ptr<ITraceProvider> provider) = 0;
    virtual std::shared_ptr<IMetricRegistry> GetMetricRegistry() const = 0;
    virtual std::shared_ptr<ITraceProvider> GetTraceProvider() const = 0;
};

} // namespace NYdb::NMetrics
