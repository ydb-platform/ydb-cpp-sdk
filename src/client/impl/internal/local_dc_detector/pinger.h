#pragma once

#include <src/client/impl/internal/internal_header.h>
#include <src/api/protos/ydb_discovery.pb.h>

#include <util/network/sock.h>
#include <util/datetime/base.h>

namespace NYdb::inline V3 {

static constexpr std::uint32_t PING_TIMEOUT_SECONDS = 5;

TDuration PingEndpoint(const Ydb::Discovery::EndpointInfo& endpoint);

} // namespace NYdb
