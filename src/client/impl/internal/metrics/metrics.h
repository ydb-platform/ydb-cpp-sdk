#pragma once

#include <vector>
#include <string>
#include <memory>

#include <cstdint>


namespace NYdb::inline V3::NMetrics {

struct TLabel {
    std::string Name;
    std::string Value;
};

using TLabels = std::vector<TLabel>;
using TBucketBounds = std::vector<double>;

class IIntGauge {
public:
    virtual void Add(std::int64_t value) = 0;
    virtual void Set(std::int64_t value) = 0;

    virtual ~IIntGauge() = default;
};

class ICounter {
public:
    virtual void Add(std::uint64_t value) = 0;

    virtual ~ICounter() = default;
};

class IRate {
public:
    virtual void Add(std::uint64_t value) = 0;

    virtual ~IRate() = default;
};

class IHistogram {
public:
    virtual void Record(double value) = 0;
    virtual ~IHistogram() = default;
};

class IMetricsProvider {
public:
    virtual std::shared_ptr<IIntGauge> IntGauge(TLabels labels) = 0;
    virtual std::shared_ptr<ICounter> Counter(TLabels labels) = 0;

    virtual std::shared_ptr<IRate> Rate(TLabels labels) = 0;

    virtual std::shared_ptr<IHistogram> HistogramCounter(TLabels labels, TBucketBounds bounds) = 0;

    virtual std::shared_ptr<IHistogram> HistogramRate(TLabels labels, TBucketBounds bounds) = 0;

    virtual ~IMetricsProvider() = default;
};

}
