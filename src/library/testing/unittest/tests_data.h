#pragma once

#include <src/library/testing/common/env.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/network/sock.h>
=======
#include <src/util/generic/noncopyable.h>
#include <src/util/generic/ptr.h>
#include <src/util/network/sock.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/network/sock.h>
>>>>>>> origin/main

#include <string>

class TInet6StreamSocket;

// set two options: SO_REUSEADDR and SO_REUSEPORT, both are required for
// correct implementation of TPortManager because of different operating systems
// incompatibility: singe SO_REUSEADDR is enough for Linux, but not enough for Darwin
template <class TSocketType>
void SetReuseAddressAndPort(const TSocketType& sock) {
    const int retAddr = SetSockOpt(sock, SOL_SOCKET, SO_REUSEADDR, 1);
    if (retAddr < 0) {
        ythrow yexception() << "can't set SO_REUSEADDR: " << LastSystemErrorText(-retAddr);
    }

#ifdef SO_REUSEPORT
    const int retPort = SetSockOpt(sock, SOL_SOCKET, SO_REUSEPORT, 1);
    if (retPort < 0) {
        ythrow yexception() << "can't set SO_REUSEPORT: " << LastSystemErrorText(-retPort);
    }
#endif
}

class TPortManager: public TNonCopyable {
public:
    TPortManager(bool reservePortsForCurrentTest = true);
    ~TPortManager();

    // Gets free TCP port
    ui16 GetPort(ui16 port = 0);

    // Gets free TCP port
    ui16 GetTcpPort(ui16 port = 0);

    // Gets free UDP port
    ui16 GetUdpPort(ui16 port = 0);

    // Gets one free port for use in both TCP and UDP protocols
    ui16 GetTcpAndUdpPort(ui16 port = 0);

    ui16 GetPortsRange(const ui16 startPort, const ui16 range);

private:
    class TPortManagerImpl;
    THolder<TPortManagerImpl> Impl_;
};

ui16 GetRandomPort();
