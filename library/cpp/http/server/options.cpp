#include "options.h"

#include <util/string/cast.h>
#include <util/digest/numeric.h>
#include <util/network/ip.h>
#include <util/network/socket.h>
#include <util/generic/hash_set.h>
#include <util/generic/yexception.h>

using TAddr = THttpServerOptions::TAddr;

static inline std::string AddrToString(const TAddr& addr) {
    return addr.Addr + ":" + ToString(addr.Port);
}

static inline TNetworkAddress ToNetworkAddr(const std::string& address, ui16 port) {
    if (address.empty() || address == std::string_view("*")) {
        return TNetworkAddress(port);
    }

    return TNetworkAddress(address.c_str(), port);
}

void THttpServerOptions::BindAddresses(TBindAddresses& ret) const {
    THashSet<std::string> check;

    for (auto addr : BindSockaddr) {
        if (!addr.Port) {
            addr.Port = Port;
        }

        const std::string straddr = AddrToString(addr);

        if (check.find(straddr) == check.end()) {
            check.insert(straddr);
            ret.push_back(ToNetworkAddr(addr.Addr, addr.Port));
        }
    }

    if (ret.empty()) {
        ret.push_back(!Host.empty() ? TNetworkAddress(Host.c_str(), Port) : TNetworkAddress(Port));
    }
}
