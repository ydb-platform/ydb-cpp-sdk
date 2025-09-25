#include "stats.h"

#include <string>

namespace NYdb::inline V3 {
namespace NSdkStats {

const NMetrics::TLabel SESSIONS_ON_KQP_HOST_LABEL = NMetrics::TLabel {"sensor", "SessionsByYdbHost"};
const NMetrics::TLabel TRANSPORT_ERRORS_BY_HOST_LABEL = NMetrics::TLabel {"sensor", "TransportErrorsByYdbHost"};
const NMetrics::TLabel GRPC_INFLIGHT_BY_HOST_LABEL = NMetrics::TLabel {"sensor", "Grpc/InFlightByYdbHost"};

std::string UnderscoreToUpperCamel(const std::string& in) {
    std::string result;
    result.reserve(in.size());

    if (in.empty()) {
        return {};
    }

    result.push_back(toupper(in[0]));

    size_t i = 1;

    while (i < in.size()) {
        if (in[i] == '_') {
            if (++i < in.size()) {
                result.push_back(toupper(in[i++]));
            } else {
                break;
            }
        } else {
            result.push_back(tolower(in[i++]));
        }
    }
    return result;
}

NMetrics::TBucketBounds ExponentialHistogram(std::uint32_t bucketsCount, double base, double scale) {
    NMetrics::TBucketBounds bounds;
    bounds.reserve(bucketsCount);

    for (std::uint32_t i = 0; i < bucketsCount; ++i) {
        bounds.push_back(scale * std::pow(base, i));
    }

    return bounds;
}

void TStatCollector::IncSessionsOnHost(const std::string& host) {
    if (auto ptr = MetricRegistryPtr_.Get()) {
        ptr->IntGauge({ DatabaseLabel_, SESSIONS_ON_KQP_HOST_LABEL, {"YdbHost", host} })->Add(1);
    }
}

void TStatCollector::DecSessionsOnHost(const std::string& host) {
    if (auto ptr = MetricRegistryPtr_.Get()) {
        ptr->IntGauge({ DatabaseLabel_, SESSIONS_ON_KQP_HOST_LABEL, {"YdbHost", host} })->Add(-1);
    }
}

void TStatCollector::IncTransportErrorsByHost(const std::string& host) {
    if (auto ptr = MetricRegistryPtr_.Get()) {
        ptr->Rate({ DatabaseLabel_, TRANSPORT_ERRORS_BY_HOST_LABEL, {"YdbHost", host} })->Add(1);
    }
}

void TStatCollector::IncGRpcInFlightByHost(const std::string& host) {
    if (auto ptr = MetricRegistryPtr_.Get()) {
        ptr->IntGauge({ DatabaseLabel_, GRPC_INFLIGHT_BY_HOST_LABEL, {"YdbHost", host} })->Add(1);
    }
}

void TStatCollector::DecGRpcInFlightByHost(const std::string& host) {
    if (auto ptr = MetricRegistryPtr_.Get()) {
        ptr->IntGauge({ DatabaseLabel_, GRPC_INFLIGHT_BY_HOST_LABEL, {"YdbHost", host} })->Add(-1);
    }
}

} // namespace NSdkStats
} // namespace NYdb
