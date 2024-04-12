#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/http/server/options.h
#include <ydb-cpp-sdk/util/network/ip.h>
#include <ydb-cpp-sdk/util/network/init.h>
#include <ydb-cpp-sdk/util/network/address.h>
#include <ydb-cpp-sdk/util/generic/size_literals.h>

#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/network/ip.h>
#include <src/util/network/init.h>
#include <src/util/network/address.h>
#include <src/util/generic/size_literals.h>

#include <src/util/datetime/base.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/http/server/options.h

class THttpServerOptions {
public:
    inline THttpServerOptions(ui16 port = 17000) noexcept
        : Port(port)
    {
    }

    using TBindAddresses = std::vector<TNetworkAddress>;
    void BindAddresses(TBindAddresses& ret) const;

    inline THttpServerOptions& AddBindAddress(const std::string& address, ui16 port) {
        const TAddr addr = {
            address,
            port,
        };

        BindSockaddr.push_back(addr);
        return *this;
    }

    inline THttpServerOptions& AddBindAddress(const std::string& address) {
        return AddBindAddress(address, 0);
    }

    inline THttpServerOptions& EnableKeepAlive(bool enable) noexcept {
        KeepAliveEnabled = enable;

        return *this;
    }

    inline THttpServerOptions& EnableCompression(bool enable) noexcept {
        CompressionEnabled = enable;

        return *this;
    }

    inline THttpServerOptions& EnableRejectExcessConnections(bool enable) noexcept {
        RejectExcessConnections = enable;

        return *this;
    }

    inline THttpServerOptions& EnableReusePort(bool enable) noexcept {
        ReusePort = enable;

        return *this;
    }

    inline THttpServerOptions& EnableReuseAddress(bool enable) noexcept {
        ReuseAddress = enable;

        return *this;
    }

    inline THttpServerOptions& SetThreads(ui32 threads) noexcept {
        nThreads = threads;

        return *this;
    }

    /// Default interface name to bind the server. Used when none of BindAddress are provided.
    inline THttpServerOptions& SetHost(const std::string& host) noexcept {
        Host = host;

        return *this;
    }

    /// Default port to bind the server. Used when none of BindAddress are provided.
    inline THttpServerOptions& SetPort(ui16 port) noexcept {
        Port = port;

        return *this;
    }

    inline THttpServerOptions& SetMaxConnections(ui32 mc = 0) noexcept {
        MaxConnections = mc;

        return *this;
    }

    inline THttpServerOptions& SetMaxQueueSize(ui32 mqs = 0) noexcept {
        MaxQueueSize = mqs;

        return *this;
    }

    inline THttpServerOptions& SetClientTimeout(const TDuration& timeout) noexcept {
        ClientTimeout = timeout;

        return *this;
    }

    inline THttpServerOptions& SetListenBacklog(int val) noexcept {
        ListenBacklog = val;

        return *this;
    }

    inline THttpServerOptions& SetOutputBufferSize(size_t val) noexcept {
        OutputBufferSize = val;

        return *this;
    }

    inline THttpServerOptions& SetMaxInputContentLength(ui64 val) noexcept {
        MaxInputContentLength = val;

        return *this;
    }

    inline THttpServerOptions& SetMaxRequestsPerConnection(size_t val) noexcept {
        MaxRequestsPerConnection = val;

        return *this;
    }

    /// Use TElasticQueue instead of TThreadPool for request queues
    inline THttpServerOptions& EnableElasticQueues(bool enable) noexcept {
        UseElasticQueues = enable;

        return *this;
    }

    inline THttpServerOptions& SetThreadsName(const std::string& listenThreadName, const std::string& requestsThreadName, const std::string& failRequestsThreadName) noexcept {
        ListenThreadName = listenThreadName;
        RequestsThreadName = requestsThreadName;
        FailRequestsThreadName = failRequestsThreadName;

        return *this;
    }

    inline THttpServerOptions& SetOneShotPoll(bool v) {
        OneShotPoll = v;

        return *this;
    }

    inline THttpServerOptions& SetListenerThreads(ui32 val) {
        nListenerThreads = val;

        return *this;
    }

    struct TAddr {
        std::string Addr;
        ui16 Port;
    };

    typedef std::vector<TAddr> TAddrs;

    bool KeepAliveEnabled = true;
    bool CompressionEnabled = false;
    bool RejectExcessConnections = false;
    bool ReusePort = false; // set SO_REUSEPORT socket option
    bool ReuseAddress = true; // set SO_REUSEADDR socket option
    TAddrs BindSockaddr;
    ui16 Port = 17000;                  // The port on which to run the web server
    std::string Host;                       // DNS entry
    const char* ServerName = "YWS/1.0"; // The Web server name to return in HTTP headers
    ui32 nThreads = 0;                  // Thread count for requests processing
    ui32 MaxQueueSize = 0;              // Max allowed request count in queue
    ui32 nFThreads = 1;
    ui32 MaxFQueueSize = 0;
    ui32 MaxConnections = 100;
    int ListenBacklog = SOMAXCONN;
    ui32 EpollMaxEvents = 1;
    TDuration ClientTimeout;
    size_t OutputBufferSize = 0;
    ui64 MaxInputContentLength = sizeof(size_t) <= 4 ? 2_GB : 64_GB;
    size_t MaxRequestsPerConnection = 0;  // If keep-alive is enabled, request limit before connection is closed
    bool UseElasticQueues = false;

    TDuration PollTimeout; // timeout of TSocketPoller::WaitT call
    TDuration ExpirationTimeout; // drop inactive connections after ExpirationTimeout (should be > 0)

    std::string ListenThreadName = "HttpListen";
    std::string RequestsThreadName = "HttpServer";
    std::string FailRequestsThreadName = "HttpServer";

    bool OneShotPoll = false;
    ui32 nListenerThreads = 1;
};
