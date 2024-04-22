#pragma once

#include <ydb-cpp-sdk/util/network/address.h>

#include <vector>

namespace NAddr {
    struct TNetworkInterface {
        std::string Name;
        IRemoteAddrRef Address;
        IRemoteAddrRef Mask;
    };

    using TNetworkInterfaceList = std::vector<TNetworkInterface>;

    TNetworkInterfaceList GetNetworkInterfaces();
}
