#define INCLUDE_YDB_INTERNAL_H
#include "local_dc_detector.h"

#include <algorithm>
#include <random>

namespace NYdb::inline V3 {

TLocalDCDetector::TLocalDCDetector(std::unique_ptr<IPinger> pinger)
    : Pinger_(std::move(pinger))
{}

std::string TLocalDCDetector::DetectLocalDC(const Ydb::Discovery::ListEndpointsResult& endpointsList) {
    auto endpointsByLocation = GroupEndpointsByLocation(endpointsList);
    SelectEndpoints(endpointsByLocation);

    if (endpointsByLocation.size() > 1) {
        Location_ = FindNearestLocation();   
        Pinger_->Reset();
    } else {
        Location_.clear();
    }
    return Location_;
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

void TLocalDCDetector::SelectEndpoints(TEndpointsByLocation& endpointsByLocation) const {
    std::mt19937 gen(std::random_device{}());
    for (auto& [location, endpoints] : endpointsByLocation) {
        if (endpoints.size() > MAX_ENDPOINTS_PER_LOCATION) {
            std::vector<TEndpointRef> sample;
            sample.reserve(MAX_ENDPOINTS_PER_LOCATION);
            std::sample(endpoints.begin(), endpoints.end(), std::back_inserter(sample), MAX_ENDPOINTS_PER_LOCATION, gen);
            endpoints.swap(sample);
        }
        for (const auto& endpoint : endpoints) {
            Pinger_->AddEndpoint(endpoint.get(), DETECT_TIMEOUT);
        }
    }
}

std::string TLocalDCDetector::FindNearestLocation() {
    try {
        return Pinger_->GetFastestLocation().GetValue(DETECT_TIMEOUT);
    } catch(...) {
        return EMPTY_LOCATION;
    }
}

} // namespace NYdb
