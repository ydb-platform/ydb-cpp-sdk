#pragma once

#include <ydb-cpp-sdk/util/system/guard.h>
#include <ydb-cpp-sdk/util/system/defaults.h>

<<<<<<< HEAD
=======
<<<<<<<< HEAD:src/util/system/rwlock.h
>>>>>>> origin/main
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/rwlock.h
#include <ydb-cpp-sdk/util/generic/ptr.h>
========
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/rwlock.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/system/rwlock.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD
=======
========
#include <ydb-cpp-sdk/util/generic/ptr.h>
>>>>>>>> origin/main:include/ydb-cpp-sdk/util/system/rwlock.h
>>>>>>> origin/main

class TRWMutex {
public:
    TRWMutex();
    ~TRWMutex();

    void AcquireRead() noexcept;
    bool TryAcquireRead() noexcept;
    void ReleaseRead() noexcept;

    void AcquireWrite() noexcept;
    bool TryAcquireWrite() noexcept;
    void ReleaseWrite() noexcept;

    void Release() noexcept;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

template <class T>
struct TReadGuardOps {
    static inline void Acquire(T* t) noexcept {
        t->AcquireRead();
    }

    static inline void Release(T* t) noexcept {
        t->ReleaseRead();
    }
};

template <class T>
struct TTryReadGuardOps: public TReadGuardOps<T> {
    static inline bool TryAcquire(T* t) noexcept {
        return t->TryAcquireRead();
    }
};

template <class T>
struct TWriteGuardOps {
    static inline void Acquire(T* t) noexcept {
        t->AcquireWrite();
    }

    static inline void Release(T* t) noexcept {
        t->ReleaseWrite();
    }
};

template <class T>
struct TTryWriteGuardOps: public TWriteGuardOps<T> {
    static inline bool TryAcquire(T* t) noexcept {
        return t->TryAcquireWrite();
    }
};

template <class T>
using TReadGuardBase = TGuard<T, TReadGuardOps<T>>;
template <class T>
using TTryReadGuardBase = TTryGuard<T, TTryReadGuardOps<T>>;

template <class T>
using TWriteGuardBase = TGuard<T, TWriteGuardOps<T>>;
template <class T>
using TTryWriteGuardBase = TTryGuard<T, TTryWriteGuardOps<T>>;

using TReadGuard = TReadGuardBase<TRWMutex>;
using TTryReadGuard = TTryReadGuardBase<TRWMutex>;

using TWriteGuard = TWriteGuardBase<TRWMutex>;
using TTryWriteGuard = TTryWriteGuardBase<TRWMutex>;
