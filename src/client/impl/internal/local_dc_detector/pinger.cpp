#define INCLUDE_YDB_INTERNAL_H
#include "pinger.h"

#include <library/cpp/neh/asio/asio.h>

namespace NYdb::inline V3 {

class TPinger::TLocationPinger : public IThreadFactory::IThreadAble {
public:
    using THandleConnect = std::function<void(const NAsio::TErrorCode& err, NAsio::IHandlingContext& handlingContext)>;

    TLocationPinger() = default;

    ~TLocationPinger() {
        if (Thread_) {
            IOService_.Abort();
            Thread_->Join();
        }
    }

    void Start(IThreadPool& pool) {
        Thread_.reset(pool.Run(this).Release());
    }

    void DoExecute() override {
        IOService_.Run();
    }

    std::size_t AddEndpointPings(const std::string& address, const std::uint32_t port, const std::chrono::seconds timeout, THandleConnect handleConnect) {
        auto& addr = Addresses_.emplace_back(address.c_str(), port);

        std::size_t pingsAmount = 0;
        for (TNetworkAddress::TIterator it = addr.Begin(); it != addr.End(); ++it) {
            auto& socket = Sockets_.emplace_back(
                std::make_unique<NAsio::TTcpSocket>(IOService_));
            auto& endpoint = Endpoints_.emplace_back(
                MakeAtomicShared<NAddr::TAddrInfo>(&*it));

            socket->AsyncConnect(endpoint, handleConnect, {timeout});
            ++pingsAmount;
        }
        return pingsAmount;
    }

private:
    NAsio::TIOService IOService_;
    std::unique_ptr<IThreadFactory::IThread> Thread_;

    std::vector<std::unique_ptr<NAsio::TTcpSocket>> Sockets_;
    std::vector<TEndpoint> Endpoints_;
    std::vector<TNetworkAddress> Addresses_;
};

TPinger::TPinger()
    : PingContext_(std::make_shared<TPingContext>())
    , ThreadPool_(std::make_unique<TAdaptiveThreadPool>())
{
    ThreadPool_->Start();
}

TPinger::~TPinger() {
    PingContext_.reset();
    ThreadPool_->Stop();
}

void TPinger::Reset() {
    PingContext_ = std::make_shared<TPingContext>();
}


void TPinger::AddEndpoint(const Ydb::Discovery::EndpointInfo& endpoint, const std::chrono::seconds timeout) {
    std::string location = endpoint.location();
    std::weak_ptr<TPingContext> pingContext = PingContext_;
    auto handleConnect = [location, pingContext](const NAsio::TErrorCode& err, NAsio::IHandlingContext& handlingContext) {
        if (auto ctx = pingContext.lock()) {
            if (!err) {
                ctx->ResultPromise.TrySetValue(location);
            } else if (ctx->PingsToFail.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                ctx->ResultPromise.TrySetException(std::make_exception_ptr(
                    std::runtime_error("All pings failed")));
            }
        }
    };

    auto [it, inserted] = PingContext_->PingerByLocation.try_emplace(location, std::make_unique<TLocationPinger>());
    auto& pinger = it->second;

    PingContext_->PingsToFail +=
        pinger->AddEndpointPings(endpoint.address(), endpoint.port(), timeout, handleConnect);
}


NThreading::TFuture<std::string> TPinger::GetFastestLocation() {
    auto future = PingContext_->ResultPromise.GetFuture();
    if (PingContext_->PingsToFail.load() > 0) {
        for (auto& [location, pinger] : PingContext_->PingerByLocation) {
            pinger->Start(*ThreadPool_);
        }
    } else {
        PingContext_->ResultPromise.TrySetException(std::make_exception_ptr(
            std::runtime_error("No pings to perform")));
    }

    return future;
}

} // namespace NYdb
