#include <ydb-cpp-sdk/util/network/init.h>

#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>

namespace {
    class TNetworkInit {
    public:
        inline TNetworkInit() {
#ifndef ROBOT_SIGPIPE
            signal(SIGPIPE, SIG_IGN);
#endif

#if defined(_win_)
    #pragma comment(lib, "ws2_32.lib")
            WSADATA wsaData;
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            Y_ASSERT(!result);
            if (result) {
                exit(-1);
            }
#endif
        }
    };
}

void InitNetworkSubSystem() {
    (void)Singleton<TNetworkInit>();
}
