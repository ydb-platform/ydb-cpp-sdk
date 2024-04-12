#pragma once

#include <ydb-cpp-sdk/util/system/defaults.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

//named sempahore
class TSemaphore {
public:
    TSemaphore(const char* name, ui32 maxFreeCount);
    ~TSemaphore();

    //Increase the semaphore counter.
    void Release() noexcept;

    //Keep a thread held while the semaphore counter is equal 0.
    void Acquire() noexcept;

    //Try to enter the semaphore gate. A non-blocking variant of Acquire.
    //Returns 'true' if the semaphore counter decreased
    bool TryAcquire() noexcept;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

//unnamed semaphore, faster, than previous
class TFastSemaphore {
public:
    TFastSemaphore(ui32 maxFreeCount);
    ~TFastSemaphore();

    void Release() noexcept;
    void Acquire() noexcept;
    bool TryAcquire() noexcept;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};
