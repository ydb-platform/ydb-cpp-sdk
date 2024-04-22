#include <ydb-cpp-sdk/library/http/server/options.h>

#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/digest/numeric.h>
#include <ydb-cpp-sdk/util/network/ip.h>
#include <ydb-cpp-sdk/util/network/socket.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <unordered_set>

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
    std::unordered_set<std::string> check;

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
