#pragma once

#include <util/datetime/base.h>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

struct TRequestData {
    TDuration Delay;
    std::string StatusLabel;
    std::uint64_t RetryAttempts;
};

class IMetricsPusher {
public:
    virtual ~IMetricsPusher() = default;
    virtual void PushRequestData(const TRequestData& requestData) = 0;
};

std::unique_ptr<IMetricsPusher> CreateOtelMetricsPusher(
    const std::string& metricsPushUrl,
    const std::string& operationType,
    const std::map<std::string, std::string>& resourceAttributes,
    const std::string& meterSchemaVersion
);
std::unique_ptr<IMetricsPusher> CreateNoopMetricsPusher();
