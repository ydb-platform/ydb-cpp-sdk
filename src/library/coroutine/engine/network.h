#pragma once

#include "iostatus.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
#include <ydb-cpp-sdk/util/network/init.h>
#include <src/util/network/iovec.h>
#include <src/util/network/nonblock.h>
#include <ydb-cpp-sdk/util/network/socket.h>
=======
#include <src/util/datetime/base.h>
#include <src/util/network/init.h>
#include <src/util/network/iovec.h>
#include <src/util/network/nonblock.h>
#include <src/util/network/socket.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TCont;

namespace NCoro {

    int SelectD(TCont* cont, SOCKET fds[], int what[], size_t nfds, SOCKET* outfd, TInstant deadline) noexcept;
    int SelectT(TCont* cont, SOCKET fds[], int what[], size_t nfds, SOCKET* outfd, TDuration timeout) noexcept;
    int SelectI(TCont* cont, SOCKET fds[], int what[], size_t nfds, SOCKET* outfd);

    int PollD(TCont* cont, SOCKET fd, int what, TInstant deadline) noexcept;
    int PollT(TCont* cont, SOCKET fd, int what, TDuration timeout) noexcept;
    int PollI(TCont* cont, SOCKET fd, int what) noexcept;

    TContIOStatus ReadVectorD(TCont* cont, SOCKET fd, TContIOVector* vec, TInstant deadline) noexcept;
    TContIOStatus ReadVectorT(TCont* cont, SOCKET fd, TContIOVector* vec, TDuration timeOut) noexcept;
    TContIOStatus ReadVectorI(TCont* cont, SOCKET fd, TContIOVector* vec) noexcept;

    TContIOStatus ReadD(TCont* cont, SOCKET fd, void* buf, size_t len, TInstant deadline) noexcept;
    TContIOStatus ReadT(TCont* cont, SOCKET fd, void* buf, size_t len, TDuration timeout) noexcept;
    TContIOStatus ReadI(TCont* cont, SOCKET fd, void* buf, size_t len) noexcept;

    TContIOStatus WriteVectorD(TCont* cont, SOCKET fd, TContIOVector* vec, TInstant deadline) noexcept;
    TContIOStatus WriteVectorT(TCont* cont, SOCKET fd, TContIOVector* vec, TDuration timeOut) noexcept;
    TContIOStatus WriteVectorI(TCont* cont, SOCKET fd, TContIOVector* vec) noexcept;

    TContIOStatus WriteD(TCont* cont, SOCKET fd, const void* buf, size_t len, TInstant deadline) noexcept;
    TContIOStatus WriteT(TCont* cont, SOCKET fd, const void* buf, size_t len, TDuration timeout) noexcept;
    TContIOStatus WriteI(TCont* cont, SOCKET fd, const void* buf, size_t len) noexcept;

    int ConnectD(TCont* cont, TSocketHolder& s, const struct addrinfo& ai, TInstant deadline) noexcept;

    int ConnectD(TCont* cont, TSocketHolder& s, const TNetworkAddress& addr, TInstant deadline) noexcept;
    int ConnectT(TCont* cont, TSocketHolder& s, const TNetworkAddress& addr, TDuration timeout) noexcept;
    int ConnectI(TCont* cont, TSocketHolder& s, const TNetworkAddress& addr) noexcept;

    int ConnectD(TCont* cont, SOCKET s, const struct sockaddr* name, socklen_t namelen, TInstant deadline) noexcept;
    int ConnectT(TCont* cont, SOCKET s, const struct sockaddr* name, socklen_t namelen, TDuration timeout) noexcept;
    int ConnectI(TCont* cont, SOCKET s, const struct sockaddr* name, socklen_t namelen) noexcept;

    int AcceptD(TCont* cont, SOCKET s, struct sockaddr* addr, socklen_t* addrlen, TInstant deadline) noexcept;
    int AcceptT(TCont* cont, SOCKET s, struct sockaddr* addr, socklen_t* addrlen, TDuration timeout) noexcept;
    int AcceptI(TCont* cont, SOCKET s, struct sockaddr* addr, socklen_t* addrlen) noexcept;

    SOCKET Socket(int domain, int type, int protocol) noexcept;
    SOCKET Socket(const struct addrinfo& ai) noexcept;
}
