#pragma once

#include "address.h"

#include <util/generic/vector.h>

namespace NAddr {
    struct TNetworkInterface {
        std::string Name;
        IRemoteAddrRef Address;
        IRemoteAddrRef Mask;
    };

    using TNetworkInterfaceList = std::vector<TNetworkInterface>;

    TNetworkInterfaceList GetNetworkInterfaces();
}
