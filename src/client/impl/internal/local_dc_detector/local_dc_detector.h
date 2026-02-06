#pragma once

#include <src/api/protos/ydb_discovery.pb.h>
#include <src/client/impl/internal/internal_header.h>
#include <src/client/impl/internal/local_dc_detector/pinger.h>

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace NYdb::inline V3 {

class TLocalDCDetector {
public:
    explicit TLocalDCDetector(std::unique_ptr<IPinger> pinger = std::make_unique<TPinger>());

    std::string DetectLocalDC(const Ydb::Discovery::ListEndpointsResult& endpoints);
    bool IsLocalDC(const std::string& location) const;

private:
    using TEndpointRef = std::reference_wrapper<const Ydb::Discovery::EndpointInfo>;
    using TEndpointsByLocation = std::unordered_map<std::string, std::vector<TEndpointRef>>;

    TEndpointsByLocation GroupEndpointsByLocation(const Ydb::Discovery::ListEndpointsResult& endpointsList) const;
    void SelectEndpoints(TEndpointsByLocation& endpointsByLocation) const;
    std::string FindNearestLocation();

private:
    static constexpr std::size_t MAX_ENDPOINTS_PER_LOCATION = 5;
    static constexpr auto DETECT_TIMEOUT = std::chrono::seconds(5);
    static constexpr auto EMPTY_LOCATION = "";

    std::unique_ptr<IPinger> Pinger_;
    std::string Location_;
};

} // namespace NYdb
