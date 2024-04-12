#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/logger/log.h>

#include <src/client/impl/ydb_internal/internal_header.h>
#include <src/client/impl/ydb_internal/common/types.h>
#include <ydb-cpp-sdk/client/common_client/ssl_credentials.h>
#include <ydb-cpp-sdk/client/types/credentials/credentials.h>
=======
#include <src/library/logger/log.h>

#include <src/client/impl/ydb_internal/internal_header.h>
#include <src/client/impl/ydb_internal/common/types.h>
#include <src/client/impl/ydb_internal/common/ssl_credentials.h>
#include <src/client/ydb_types/credentials/credentials.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {

class IConnectionsParams {
public:
    virtual ~IConnectionsParams() = default;
    virtual std::string GetEndpoint() const = 0;
    virtual size_t GetNetworkThreadsNum() const = 0;
    virtual size_t GetClientThreadsNum() const = 0;
    virtual size_t GetMaxQueuedResponses() const = 0;
    virtual TSslCredentials GetSslCredentials() const = 0;
    virtual std::string GetDatabase() const = 0;
    virtual std::shared_ptr<ICredentialsProviderFactory> GetCredentialsProviderFactory() const = 0;
    virtual EDiscoveryMode GetDiscoveryMode() const = 0;
    virtual size_t GetMaxQueuedRequests() const = 0;
    virtual NYdbGrpc::TTcpKeepAliveSettings GetTcpKeepAliveSettings() const = 0;
    virtual bool GetDrinOnDtors() const = 0;
    virtual TBalancingSettings GetBalancingSettings() const = 0;
    virtual TDuration GetGRpcKeepAliveTimeout() const = 0;
    virtual bool GetGRpcKeepAlivePermitWithoutCalls() const = 0;
    virtual TDuration GetSocketIdleTimeout() const = 0;
    virtual const TLog& GetLog() const = 0;
    virtual ui64 GetMemoryQuota() const = 0;
    virtual ui64 GetMaxInboundMessageSize() const = 0;
    virtual ui64 GetMaxOutboundMessageSize() const = 0;
    virtual ui64 GetMaxMessageSize() const = 0;
};

} // namespace NYdb
