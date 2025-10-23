#pragma once

#include <src/api/protos/ydb_discovery.pb.h>
#include <src/client/impl/internal/internal_header.h>
#include <src/client/impl/internal/local_dc_detector/pinger.h>

#include <algorithm>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace NYdb::inline V3 {

class TLocalDCDetector {
public:
    using TPinger = std::function<TDuration(const Ydb::Discovery::EndpointInfo& endpoint)>;
    explicit TLocalDCDetector(TPinger pingEndpoint = PingEndpoint);

    void DetectLocalDC(const Ydb::Discovery::ListEndpointsResult& endpoints);
    bool IsLocalDC(const std::string& location) const;

private:
    using TEndpoint = Ydb::Discovery::EndpointInfo;
    using TEndpointRef = std::reference_wrapper<const TEndpoint>;
    using TEndpointsByLocation = std::unordered_map<std::string, std::vector<TEndpointRef>>;
    using TMeasureResult = std::pair<std::string, std::uint64_t>;

    TEndpointsByLocation GroupEndpointsByLocation(const Ydb::Discovery::ListEndpointsResult& endpointsList) const;
    void SampleEndpoints(TEndpointsByLocation& endpointsByLocation) const;
    std::uint64_t MeasureLocationRtt(const std::vector<TEndpointRef>& endpoints) const;
    std::string FindNearestLocation(const TEndpointsByLocation& endpointsByLocation);

private:
    static constexpr std::size_t MAX_ENDPOINTS_PER_LOCATION = 3;
    static constexpr std::size_t PING_COUNT = 2 * MAX_ENDPOINTS_PER_LOCATION;

    TPinger PingEndpoint_;
    std::string Location_;
};

} // namespace NYdb
