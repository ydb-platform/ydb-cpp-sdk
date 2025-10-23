#define INCLUDE_YDB_INTERNAL_H
#include "pinger.h"

namespace NYdb::inline V3 {

TDuration PingEndpoint(const Ydb::Discovery::EndpointInfo& endpoint) {
    try {
        TNetworkAddress addr(endpoint.address().data(), static_cast<ui16>(endpoint.port()));
        auto start = TInstant::Now();
        TSocket sock(addr, TDuration::Seconds(PING_TIMEOUT_SECONDS));
        return TInstant::Now() - start;
    } catch (...) {
       return TDuration::Max();
    }
}

} // namespace NYdb