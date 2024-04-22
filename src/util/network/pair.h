#pragma once

#include <ydb-cpp-sdk/util/network/init.h>

int SocketPair(SOCKET socks[2], bool overlapped, bool cloexec = false);

static inline int SocketPair(SOCKET socks[2]) {
    return SocketPair(socks, false, false);
}
