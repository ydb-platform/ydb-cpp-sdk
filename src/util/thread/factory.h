#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/thread/factory.h
#include <ydb-cpp-sdk/util/generic/ptr.h>
========
#include <src/util/generic/ptr.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/thread/factory.h
#include <functional>

class IThreadFactory {
public:
    class IThreadAble {
    public:
        inline IThreadAble() noexcept = default;

        virtual ~IThreadAble() = default;

        inline void Execute() {
            DoExecute();
        }

    private:
        virtual void DoExecute() = 0;
    };

    class IThread {
        friend class IThreadFactory;

    public:
        inline IThread() noexcept = default;

        virtual ~IThread() = default;

        inline void Join() noexcept {
            DoJoin();
        }

    private:
        inline void Run(IThreadAble* func) {
            DoRun(func);
        }

    private:
        // it's actually DoStart
        virtual void DoRun(IThreadAble* func) = 0;
        virtual void DoJoin() noexcept = 0;
    };

    inline IThreadFactory() noexcept = default;

    virtual ~IThreadFactory() = default;

    // XXX: rename to Start
    inline THolder<IThread> Run(IThreadAble* func) {
        THolder<IThread> ret(DoCreate());

        ret->Run(func);

        return ret;
    }

    THolder<IThread> Run(const std::function<void()>& func);

private:
    virtual IThread* DoCreate() = 0;
};

IThreadFactory* SystemThreadFactory();
void SetSystemThreadFactory(IThreadFactory* pool);
