#pragma once

#include <ydb-cpp-sdk/util/network/socket.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/generic/ptr.h>
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
>>>>>>> origin/main

class TSocketPoller {
public:
    TSocketPoller();
    ~TSocketPoller();

    void WaitRead(SOCKET sock, void* cookie);
    void WaitWrite(SOCKET sock, void* cookie);
    void WaitReadWrite(SOCKET sock, void* cookie);
    void WaitRdhup(SOCKET sock, void* cookie);

    void WaitReadOneShot(SOCKET sock, void* cookie);
    void WaitWriteOneShot(SOCKET sock, void* cookie);
    void WaitReadWriteOneShot(SOCKET sock, void* cookie);

    void WaitReadWriteEdgeTriggered(SOCKET sock, void* cookie);
    void RestartReadWriteEdgeTriggered(SOCKET sock, void* cookie, bool empty = true);

    void Unwait(SOCKET sock);

    size_t WaitD(void** events, size_t len, const TInstant& deadLine);

    inline size_t WaitT(void** events, size_t len, const TDuration& timeOut) {
        return WaitD(events, len, timeOut.ToDeadLine());
    }

    inline size_t WaitI(void** events, size_t len) {
        return WaitD(events, len, TInstant::Max());
    }

    inline void* WaitD(const TInstant& deadLine) {
        void* ret;

        if (WaitD(&ret, 1, deadLine)) {
            return ret;
        }

        return nullptr;
    }

    inline void* WaitT(const TDuration& timeOut) {
        return WaitD(timeOut.ToDeadLine());
    }

    inline void* WaitI() {
        return WaitD(TInstant::Max());
    }

private:
    class TImpl;
    THolder<TImpl> Impl_;
};
