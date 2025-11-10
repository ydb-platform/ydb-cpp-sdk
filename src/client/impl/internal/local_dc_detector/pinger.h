#pragma once

#include <src/api/protos/ydb_discovery.pb.h>
#include <src/client/impl/internal/internal_header.h>

#include <library/cpp/threading/future/future.h>

#include <util/thread/pool.h>

#include <chrono>

namespace NYdb::inline V3 {

class IPinger {
public:
    virtual ~IPinger() = default;

    virtual void Reset() = 0;
    virtual void AddEndpoint(const Ydb::Discovery::EndpointInfo& endpoint, const std::chrono::seconds timeout) = 0;
    virtual NThreading::TFuture<std::string> GetFastestLocation() = 0;
};

class TPinger : public IPinger {
public:
    TPinger();
    ~TPinger() override;

    void Reset() override;
    void AddEndpoint(const Ydb::Discovery::EndpointInfo& endpoint, const std::chrono::seconds timeout) override;

    NThreading::TFuture<std::string> GetFastestLocation() override;

private:
    class TLocationPinger;

    struct TPingContext {
        std::unordered_map<std::string, std::unique_ptr<TLocationPinger>> PingerByLocation;
        std::atomic<size_t> PingsToFail = 0;
        NThreading::TPromise<std::string> ResultPromise = NThreading::NewPromise<std::string>();
    };

    std::shared_ptr<TPingContext> PingContext_;
    std::unique_ptr<TAdaptiveThreadPool> ThreadPool_;
};

} // namespace NYdb
