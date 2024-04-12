#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/thread/pool.h>
=======
#include <src/util/thread/pool.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <memory>

namespace NYdb {

inline std::unique_ptr<IThreadPool> CreateThreadPool(size_t threads) {
    std::unique_ptr<IThreadPool> queue;
    if (threads) {
        queue.reset(new TThreadPool(TThreadPool::TParams().SetBlocking(true).SetCatching(false)));
    } else {
        queue.reset(new TAdaptiveThreadPool());
    }
    return queue;
}

} // namespace NYdb
