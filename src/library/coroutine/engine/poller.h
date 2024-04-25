#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <ydb-cpp-sdk/util/network/socket.h>
#include <src/util/network/pollerimpl.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
<<<<<<< HEAD
=======
#include <src/util/generic/ptr.h>

#include <src/util/network/socket.h>
#include <src/util/network/pollerimpl.h>
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

enum class EContPoller {
    Default /* "default" */,
    Select /* "select" */,
    Poll /* "poll" */,
    Epoll /* "epoll" */,
    Kqueue /* "kqueue" */
};

class IPollerFace {
public:
    struct TChange {
        SOCKET Fd;
        void* Data;
        ui16 Flags;
    };

    struct TEvent {
        void* Data;
        int Status;
        ui16 Filter;
    };

    using TEvents = std::vector<TEvent>;

    virtual ~IPollerFace() {
    }

    void Set(void* ptr, SOCKET fd, ui16 flags) {
        const TChange c = {fd, ptr, flags};

        Set(c);
    }

    virtual void Set(const TChange& change) = 0;
    virtual void Wait(TEvents& events, TInstant deadLine) = 0;
    virtual EContPoller PollEngine() const = 0;

    static THolder<IPollerFace> Default();
    static THolder<IPollerFace> Construct(std::string_view name);
    static THolder<IPollerFace> Construct(EContPoller poller);
};
