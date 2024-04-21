#include "factory.h"

#include <src/util/system/thread.h>
#include <src/util/generic/singleton.h>

using IThread = IThreadFactory::IThread;

namespace {
    class TSystemThreadFactory: public IThreadFactory {
    public:
        class TPoolThread: public IThread {
        public:
            ~TPoolThread() override {
                if (Thr_) {
                    Thr_->Detach();
                }
            }

            void DoRun(IThreadAble* func) override {
                Thr_.reset(new TThread(ThreadProc, func));

                Thr_->Start();
            }

            void DoJoin() noexcept override {
                if (!Thr_) {
                    return;
                }

                Thr_->Join();
                Thr_.reset();
            }

        private:
            static void* ThreadProc(void* func) {
                ((IThreadAble*)(func))->Execute();

                return nullptr;
            }

        private:
            std::unique_ptr<TThread> Thr_;
        };

        inline TSystemThreadFactory() noexcept {
        }

        IThread* DoCreate() override {
            return new TPoolThread;
        }
    };

    class TThreadFactoryFuncObj: public IThreadFactory::IThreadAble {
    public:
        TThreadFactoryFuncObj(const std::function<void()>& func)
            : Func(func)
        {
        }
        void DoExecute() override {
            std::unique_ptr<TThreadFactoryFuncObj> self(this);
            Func();
        }

    private:
        std::function<void()> Func;
    };
}

std::unique_ptr<IThread> IThreadFactory::Run(const std::function<void()>& func) {
    std::unique_ptr<IThread> ret(DoCreate());

    ret->Run(new ::TThreadFactoryFuncObj(func));

    return ret;
}

static IThreadFactory* SystemThreadPoolImpl() {
    return Singleton<TSystemThreadFactory>();
}

static IThreadFactory* systemPool = nullptr;

IThreadFactory* SystemThreadFactory() {
    if (systemPool) {
        return systemPool;
    }

    return SystemThreadPoolImpl();
}

void SetSystemThreadFactory(IThreadFactory* pool) {
    systemPool = pool;
}
