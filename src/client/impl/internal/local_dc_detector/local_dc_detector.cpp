#define INCLUDE_YDB_INTERNAL_H
#include "local_dc_detector.h"

namespace NYdb::inline V3 {

TLocalDCDetector::TLocalDCDetector(TPinger pingEndpoint)
    : PingEndpoint_(std::move(pingEndpoint))
{}

void TLocalDCDetector::DetectLocalDC(const Ydb::Discovery::ListEndpointsResult& endpointsList) {
    auto endpointsByLocation = GroupEndpointsByLocation(endpointsList);
    SampleEndpoints(endpointsByLocation);

    if (endpointsByLocation.size() > 1) {
        Location_ = FindNearestLocation(endpointsByLocation);   
    } else {
        Location_.clear();
    }
}

bool TLocalDCDetector::IsLocalDC(const std::string& location) const {
    return Location_.empty() || Location_ == location;
}

TLocalDCDetector::TEndpointsByLocation TLocalDCDetector::GroupEndpointsByLocation(const Ydb::Discovery::ListEndpointsResult& endpointsList) const {
    TEndpointsByLocation endpointsByLocation;
    for (const auto& endpoint : endpointsList.endpoints()) {
        endpointsByLocation[endpoint.location()].emplace_back(endpoint);
    }
    return endpointsByLocation;
}

void TLocalDCDetector::SampleEndpoints(TEndpointsByLocation& endpointsByLocation) const {
    std::mt19937 gen(std::random_device{}());
    for (auto& [location, endpoints] : endpointsByLocation) {
        if (endpoints.size() > MAX_ENDPOINTS_PER_LOCATION) {
            std::vector<TEndpointRef> sample;
            sample.reserve(MAX_ENDPOINTS_PER_LOCATION);
            std::sample(endpoints.begin(), endpoints.end(), std::back_inserter(sample), MAX_ENDPOINTS_PER_LOCATION, gen);
            endpoints.swap(sample);
        }
    }
}

std::uint64_t TLocalDCDetector::MeasureLocationRtt(const std::vector<TEndpointRef>& endpoints) const {
    if (endpoints.empty()) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    std::vector<std::uint64_t> timings;
    timings.reserve(PING_COUNT);
    for (size_t i = 0; i < PING_COUNT; ++i) {
        const auto& ep = endpoints[i % endpoints.size()].get();
        timings.push_back(PingEndpoint_(ep).MicroSeconds());
    }
    std::sort(timings.begin(), timings.end());

    return std::midpoint(timings[(PING_COUNT - 1) / 2], timings[PING_COUNT / 2]);
}


std::string TLocalDCDetector::FindNearestLocation(const TEndpointsByLocation& endpointsByLocation) {
    auto minRtt = std::numeric_limits<std::uint64_t>::max();
    std::string nearestLocation;
    for (const auto& [location, endpoints] : endpointsByLocation) {
        auto rtt = MeasureLocationRtt(endpoints);
        if (rtt < minRtt) {
            minRtt = rtt;
            nearestLocation = location;
        }
    }
    return nearestLocation;
}

} // namespace NYdb
