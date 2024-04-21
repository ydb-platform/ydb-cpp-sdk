#include <ydb-cpp-sdk/util/system/defaults.h>

#include "event.h"

#include <condition_variable>
#include <mutex>

#ifdef _win_
    #include "winint.h"
#endif

#include <atomic>

class TSystemEvent::TEvImpl: public TAtomicRefCount<TSystemEvent::TEvImpl> {
public:
#ifdef _win_
    inline TEvImpl(ResetMode rmode) {
        cond = CreateEvent(nullptr, rmode == rManual ? true : false, false, nullptr);
    }

    inline ~TEvImpl() {
        CloseHandle(cond);
    }

    inline void Reset() noexcept {
        ResetEvent(cond);
    }

    inline void Signal() noexcept {
        SetEvent(cond);
    }

    inline bool WaitD(TInstant deadLine) noexcept {
        if (deadLine == TInstant::Max()) {
            return WaitForSingleObject(cond, INFINITE) == WAIT_OBJECT_0;
        }

        const TInstant now = Now();

        if (now < deadLine) {
            //TODO
            return WaitForSingleObject(cond, (deadLine - now).MilliSeconds()) == WAIT_OBJECT_0;
        }

        return (WaitForSingleObject(cond, 0) == WAIT_OBJECT_0);
    }
#else
    inline TEvImpl(ResetMode rmode)
        : Manual(rmode == rManual ? true : false)
    {
    }

    inline void Signal() noexcept {
        if (Manual && Signaled.load(std::memory_order_acquire)) {
            return; // shortcut
        }

        {
            std::lock_guard guard(Mutex);
            Signaled.store(true, std::memory_order_release);
        }

        if (Manual) {
            Cond.notify_all();
        } else {
            Cond.notify_one();
        }
    }

    inline void Reset() noexcept {
        Signaled.store(false, std::memory_order_release);
    }

    inline bool WaitD(TInstant deadLine) noexcept {
        if (Manual && Signaled.load(std::memory_order_acquire)) {
            return true; // shortcut
        }

        bool resSignaled = true;

        std::unique_lock ulock(Mutex);
        while (!Signaled.load(std::memory_order_acquire)) {
            auto timeout = std::chrono::nanoseconds(std::min(deadLine.NanoSeconds(),
                                                    static_cast<ui64>(std::chrono::nanoseconds::max().count())));
            if (Cond.wait_until(ulock, std::chrono::time_point<std::chrono::system_clock>(timeout)) == std::cv_status::timeout) {
                resSignaled = Signaled.load(std::memory_order_acquire); // timed out, but Signaled could have been set

                break;
            }
        }

        if (!Manual) {
            Signaled.store(false, std::memory_order_release);
        }

        return resSignaled;
    }
#endif

private:
#ifdef _win_
    HANDLE cond;
#else
    std::condition_variable Cond;
    std::mutex Mutex;
    std::atomic<bool> Signaled = false;
    bool Manual;
#endif
};

TSystemEvent::TSystemEvent(ResetMode rmode)
    : EvImpl_(new TEvImpl(rmode))
{
}

TSystemEvent::TSystemEvent(const TSystemEvent& other) noexcept
    : EvImpl_(other.EvImpl_)
{
}

TSystemEvent& TSystemEvent::operator=(const TSystemEvent& other) noexcept {
    EvImpl_ = other.EvImpl_;
    return *this;
}

TSystemEvent::~TSystemEvent() = default;

void TSystemEvent::Reset() noexcept {
    EvImpl_->Reset();
}

void TSystemEvent::Signal() noexcept {
    EvImpl_->Signal();
}

bool TSystemEvent::WaitD(TInstant deadLine) noexcept {
    return EvImpl_->WaitD(deadLine);
}
