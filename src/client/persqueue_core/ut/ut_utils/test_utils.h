#pragma once
<<<<<<<< HEAD:src/client/persqueue_core/ut/ut_utils/test_utils.h
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/generic/size_literals.h>
========
#include <src/util/generic/ptr.h>
#include <src/util/generic/size_literals.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_persqueue_core/ut/ut_utils/test_utils.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_persqueue_core/ut/ut_utils/test_utils.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
#include <src/library/threading/chunk_queue/queue.h>
#include <src/util/generic/overloaded.h>
#include <src/library/testing/unittest/registar.h>

#include "sdk_test_setup.h"

namespace NPersQueue {

using namespace NThreading;
using namespace NYdb::NPersQueue;
using namespace NKikimr;
using namespace NKikimr::NPersQueueTests;


struct TWriteResult {
    bool Ok = false;
    // No acknowledgement is expected from a writer under test
    bool NoWait = false;
    std::string ResponseDebugString = std::string();
};

struct TAcknowledgableMessage {
    std::string Value;
    ui64 SequenceNumber;
    TInstant CreatedAt;
    TPromise<TWriteResult> AckPromise;
};

class IClientEventLoop {
protected:
    std::atomic_bool MayStop;
    std::atomic_bool MustStop;
    bool Stopped = false;
    std::unique_ptr<TThread> Thread;
    TLog Log;

public:
    IClientEventLoop()
    : MayStop()
    , MustStop()
    , MessageBuffer()
    {}

    void AllowStop() {
        MayStop = true;
    }

    void WaitForStop() {
        if (!Stopped) {
            Log << TLOG_INFO << "Wait for writer to die on itself";
            Thread->Join();
            Log << TLOG_INFO << "Client write event loop stopped";
        }
        Stopped = true;
    }

    virtual ~IClientEventLoop() {
        MustStop = true;
        if (!Stopped) {
            Log << TLOG_INFO << "Wait for client write event loop to stop";
            Thread->Join();
            Log << TLOG_INFO << "Client write event loop stopped";
        }
        Stopped = true;
    }

    TManyOneQueue<TAcknowledgableMessage> MessageBuffer;

};

} // namespace NPersQueue
