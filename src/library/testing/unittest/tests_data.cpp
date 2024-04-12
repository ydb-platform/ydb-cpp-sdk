#include "tests_data.h"
#include "registar.h"

#include <src/library/testing/common/env_var.h>
#include <src/library/testing/common/network.h>

#include <mutex>

class TPortManager::TPortManagerImpl {
public:
    TPortManagerImpl(bool reservePortsForCurrentTest)
        : EnableReservePortsForCurrentTest(reservePortsForCurrentTest)
        , DisableRandomPorts(!NUtils::GetEnv("NO_RANDOM_PORTS").empty())
    {
    }

    ui16 GetPort(ui16 port) {
        if (port && DisableRandomPorts) {
            return port;
        }

        TAtomicSharedPtr<NTesting::IPort> holder(NTesting::GetFreePort().Release());
        ReservePortForCurrentTest(holder);

        std::lock_guard g(Lock);
        ReservedPorts.push_back(holder);
        return holder->Get();
    }

    ui16 GetUdpPort(ui16 port) {
        return GetPort(port);
    }

    ui16 GetTcpPort(ui16 port) {
        return GetPort(port);
    }

    ui16 GetTcpAndUdpPort(ui16 port) {
        return GetPort(port);
    }

    ui16 GetPortsRange(const ui16 startPort, const ui16 range) {
        Y_UNUSED(startPort);
        auto ports = NTesting::NLegacy::GetFreePortsRange(range);
        ui16 first = ports[0];
        std::lock_guard g(Lock);
        for (auto& port : ports) {
            ReservedPorts.emplace_back(port.Release());
            ReservePortForCurrentTest(ReservedPorts.back());
        }
        return first;
    }

private:
    void ReservePortForCurrentTest(const TAtomicSharedPtr<NTesting::IPort>& portGuard) {
        if (EnableReservePortsForCurrentTest) {
            TTestBase* currentTest = NUnitTest::NPrivate::GetCurrentTest();
            if (currentTest != nullptr) {
                currentTest->RunAfterTest([guard = portGuard]() mutable {
                    guard = nullptr; // remove reference for allocated port
                });
            }
        }
    }

private:
    std::mutex Lock;
    std::vector<TAtomicSharedPtr<NTesting::IPort>> ReservedPorts;
    const bool EnableReservePortsForCurrentTest;
    const bool DisableRandomPorts;
};

TPortManager::TPortManager(bool reservePortsForCurrentTest)
    : Impl_(new TPortManagerImpl(reservePortsForCurrentTest))
{
}

TPortManager::~TPortManager() {
}

ui16 TPortManager::GetPort(ui16 port) {
    return Impl_->GetTcpPort(port);
}

ui16 TPortManager::GetTcpPort(ui16 port) {
    return Impl_->GetTcpPort(port);
}

ui16 TPortManager::GetUdpPort(ui16 port) {
    return Impl_->GetUdpPort(port);
}

ui16 TPortManager::GetTcpAndUdpPort(ui16 port) {
    return Impl_->GetTcpAndUdpPort(port);
}

ui16 TPortManager::GetPortsRange(const ui16 startPort, const ui16 range) {
    return Impl_->GetPortsRange(startPort, range);
}

ui16 GetRandomPort() {
    TPortManager* pm = Singleton<TPortManager>(false);
    return pm->GetPort();
}
