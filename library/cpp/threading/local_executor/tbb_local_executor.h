#pragma once

#include "local_executor.h"

#define __TBB_TASK_ISOLATION 1
#define __TBB_NO_IMPLICIT_LINKAGE 1

#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/task_arena.h>
#include <oneapi/tbb/task_group.h>

#include <functional>

namespace NPar {

template <bool RespectTls = false>
class TTbbLocalExecutor final : public ILocalExecutor {
public:
    explicit TTbbLocalExecutor(int threadCount)
        : TbbArena_(threadCount)
    {
    }
    ~TTbbLocalExecutor() noexcept override {
    }

    int GetWorkerThreadId() const noexcept override;
    int GetThreadCount() const noexcept override;

    void Exec(TIntrusivePtr<ILocallyExecutable> exec, int id, int flags) override;
    void ExecRange(TIntrusivePtr<ILocallyExecutable> exec, int firstId, int lastId, int flags) override;

    void Initialize();
    void Wait();

private:
    using TFunction = std::function<void(int)>;

    void SubmitAsyncTasks(TFunction exec, int firstId, int lastId);

private:
    mutable oneapi::tbb::task_arena TbbArena_;
    oneapi::tbb::task_group Group_;
};

} // namespace NPar
