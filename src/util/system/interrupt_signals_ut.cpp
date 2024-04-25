#include "interrupt_signals.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/datetime/base.h>
>>>>>>> origin/main

#include <src/library/testing/unittest/registar.h>

#include <atomic>

#ifdef _win_
    #include <windows.h>
#endif

Y_UNIT_TEST_SUITE(TTestInterruptSignals) {
    static std::atomic<size_t> HandledSigNum = 0;

    static void Handler(int signum) {
        HandledSigNum.store(signum);
    }

    Y_UNIT_TEST(Test1) {
        SetInterruptSignalsHandler(Handler);
#ifdef _win_
        // TODO: unfortunately GenerateConsoleCtrlEvent fails under Wine
        /*
         for (auto [winEvent, posixSigNum] : {
                std::make_pair(CTRL_C_EVENT, SIGINT),
                std::make_pair(CTRL_BREAK_EVENT, SIGTERM)
            })
        {
            if (!GenerateConsoleCtrlEvent(winEvent, 0)) {
                UNIT_FAIL("GenerateConsoleCtrlEvent failed: " << LastSystemErrorText());
            }
            Sleep(TDuration::MilliSeconds(100));
            UNIT_ASSERT_VALUES_EQUAL(HandledSigNum.load(), posixSigNum);
        }
        */
        for (int signum : {SIGINT, SIGTERM}) {
#else
        for (int signum : {SIGINT, SIGTERM, SIGHUP}) {
#endif
            std::raise(signum);
            Sleep(TDuration::MilliSeconds(100)); // give it time to handle an async signal
            UNIT_ASSERT_VALUES_EQUAL(HandledSigNum.load(), signum);
        }
    }
}
