#pragma once

#include <ydb-cpp-sdk/util/network/address.h>

#include <vector>
#include <memory>

namespace NAddr {
    struct TNetworkInterface {
        std::string Name;
        IRemoteAddrRef Address;
        IRemoteAddrRef Mask;
    };

    using TNetworkInterfaceList = std::vector<TNetworkInterface>;

    TNetworkInterfaceList GetNetworkInterfaces();
}
