#pragma once

#include <ydb-cpp-sdk/util/network/address.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/str_stl.h>
=======
#include <src/util/str_stl.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/str_stl.h>
>>>>>>> origin/main

//some equivalent boost::asio::ip::endpoint (easy for using pair ip:port)
class TEndpoint {
public:
    using TAddrRef = NAddr::IRemoteAddrRef;

    TEndpoint(const TAddrRef& addr);
    TEndpoint();

    inline const TAddrRef& Addr() const noexcept {
        return Addr_;
    }
    inline const sockaddr* SockAddr() const {
        return Addr_->Addr();
    }
    inline socklen_t SockAddrLen() const {
        return Addr_->Len();
    }

    inline bool IsIpV4() const {
        return Addr_->Addr()->sa_family == AF_INET;
    }
    inline bool IsIpV6() const {
        return Addr_->Addr()->sa_family == AF_INET6;
    }
    inline bool IsUnix() const {
        return Addr_->Addr()->sa_family == AF_UNIX;
    }

    inline std::string IpToString() const {
        return NAddr::PrintHost(*Addr_);
    }

    void SetPort(ui16 port);
    ui16 Port() const noexcept;

    size_t Hash() const;

private:
    TAddrRef Addr_;
};

template <>
struct THash<TEndpoint> {
    inline size_t operator()(const TEndpoint& ep) const {
        return ep.Hash();
    }
};

inline bool operator==(const TEndpoint& l, const TEndpoint& r) {
    try {
        return NAddr::IsSame(*l.Addr(), *r.Addr()) && l.Port() == r.Port();
    } catch (...) {
        return false;
    }
}
