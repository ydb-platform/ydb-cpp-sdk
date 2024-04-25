#pragma once

<<<<<<< HEAD
=======
<<<<<<<< HEAD:src/util/system/event.h
>>>>>>> origin/main
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/event.h
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/generic/ptr.h>
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/event.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/system/event.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD
=======
========
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
>>>>>>>> origin/main:include/ydb-cpp-sdk/util/system/event.h
>>>>>>> origin/main

struct TEventResetType {
    enum ResetMode {
        rAuto,   // the state will be nonsignaled after Wait() returns
        rManual, // we need call Reset() to set the state to nonsignaled.
    };
};

/**
 * DEPRECATED!
 *
 * Use TAutoEvent, TManualEvent for the direct replacement.
 * Use TManualEvent to prevent SEGFAULT (http://nga.at.yandex-team.ru/5772).
 */
class TSystemEvent: public TEventResetType {
public:
    TSystemEvent(ResetMode rmode = rManual);
    TSystemEvent(const TSystemEvent& other) noexcept;
    TSystemEvent& operator=(const TSystemEvent& other) noexcept;

    ~TSystemEvent();

    void Reset() noexcept;
    void Signal() noexcept;

    /*
     * return true if signaled, false if timed out.
     */
    bool WaitD(TInstant deadLine) noexcept;

    /*
     * return true if signaled, false if timed out.
     */
    inline bool WaitT(TDuration timeOut) noexcept {
        return WaitD(timeOut.ToDeadLine());
    }

    /*
     * wait infinite time
     */
    inline void WaitI() noexcept {
        WaitD(TInstant::Max());
    }

    //return true if signaled, false if timed out.
    inline bool Wait(ui32 timer) noexcept {
        return WaitT(TDuration::MilliSeconds(timer));
    }

    inline bool Wait() noexcept {
        WaitI();

        return true;
    }

private:
    class TEvImpl;
    TIntrusivePtr<TEvImpl> EvImpl_;
};

class TAutoEvent: public TSystemEvent {
public:
    TAutoEvent()
        : TSystemEvent(TSystemEvent::rAuto)
    {
    }

private:
    void Reset() noexcept;
};

/**
 * Prevents from a "shortcut problem" (see http://nga.at.yandex-team.ru/5772): if Wait will be called after Signaled
 * flag set to true in Signal method but before CondVar.BroadCast - Wait will shortcut (without actual wait on condvar).
 * If Wait thread will destruct event - Signal thread will do broadcast on a destructed CondVar.
 */
class TManualEvent {
public:
    TManualEvent()
        : Ev(TEventResetType::rManual)
    {
    }

    void Reset() noexcept {
        TSystemEvent{Ev}.Reset();
    }

    void Signal() noexcept {
        TSystemEvent{Ev}.Signal();
    }

    /** return true if signaled, false if timed out. */
    bool WaitD(TInstant deadLine) noexcept {
        return TSystemEvent{Ev}.WaitD(deadLine);
    }

    /** return true if signaled, false if timed out. */
    inline bool WaitT(TDuration timeOut) noexcept {
        return TSystemEvent{Ev}.WaitT(timeOut);
    }

    /** Wait infinite time */
    inline void WaitI() noexcept {
        TSystemEvent{Ev}.WaitI();
    }

    /** return true if signaled, false if timed out. */
    inline bool Wait(ui32 timer) noexcept {
        return TSystemEvent{Ev}.Wait(timer);
    }

    inline bool Wait() noexcept {
        return TSystemEvent{Ev}.Wait();
    }

private:
    TSystemEvent Ev;
};
