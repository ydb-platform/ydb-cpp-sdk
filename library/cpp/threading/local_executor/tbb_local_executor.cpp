#include "tbb_local_executor.h"

#include <algorithm>

namespace NPar {

template <bool RespectTls>
void TTbbLocalExecutor<RespectTls>::Initialize() {
    TbbArena_.initialize();
}

template <bool RespectTls>
void TTbbLocalExecutor<RespectTls>::Wait() {
    TbbArena_.execute([this] {
        Group_.wait();
    });
}

template <bool RespectTls>
void TTbbLocalExecutor<RespectTls>::SubmitAsyncTasks(TFunction exec, int firstId, int lastId) {
    for (int id = firstId; id < lastId; ++id) {
        // task_group::run may leave an asynchronously submitted task dormant
        // until somebody waits on the group.  An executor has to wake a worker
        // even when no more work arrives, so enqueue the task into the arena
        // explicitly while still attaching it to the group for Wait().
        TbbArena_.enqueue([exec, id] {
            exec(id);
        }, Group_);
    }
}

template <bool RespectTls>
int TTbbLocalExecutor<RespectTls>::GetThreadCount() const noexcept {
    return std::max(TbbArena_.max_concurrency() - 1, 0);
}

template <bool RespectTls>
int TTbbLocalExecutor<RespectTls>::GetWorkerThreadId() const noexcept {
    return TbbArena_.execute([] {
        return oneapi::tbb::this_task_arena::current_thread_index();
    });
}

template <bool RespectTls>
void TTbbLocalExecutor<RespectTls>::Exec(TIntrusivePtr<ILocallyExecutable> exec, int id, int flags) {
    if (flags & WAIT_COMPLETE) {
        exec->LocalExec(id);
        return;
    }

    TbbArena_.execute([this, exec = std::move(exec), id] {
        SubmitAsyncTasks([exec](int taskId) {
            exec->LocalExec(taskId);
        }, id, id + 1);
    });
}

template <bool RespectTls>
void TTbbLocalExecutor<RespectTls>::ExecRange(
        TIntrusivePtr<ILocallyExecutable> exec, int firstId, int lastId, int flags) {
    if (flags & WAIT_COMPLETE) {
        TbbArena_.execute([exec = std::move(exec), firstId, lastId] {
            auto run = [exec, firstId, lastId] {
                oneapi::tbb::parallel_for(firstId, lastId, [exec](int id) {
                    exec->LocalExec(id);
                });
            };
            if constexpr (RespectTls) {
                oneapi::tbb::this_task_arena::isolate(std::move(run));
            } else {
                run();
            }
        });
        return;
    }

    TbbArena_.execute([this, exec = std::move(exec), firstId, lastId] {
        SubmitAsyncTasks([exec](int id) {
            exec->LocalExec(id);
        }, firstId, lastId);
    });
}

template class TTbbLocalExecutor<true>;
template class TTbbLocalExecutor<false>;

} // namespace NPar
