#include <ydb-cpp-sdk/client/types/executor/executor.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>

namespace NYdb::inline V3 {
namespace {

TEST(TTbbExecutorTest, SingleWorkerDrainsPostedTasksOnStop) {
    auto executor = CreateThreadPoolExecutor(1);
    executor->Start();

    std::atomic<std::size_t> completed = 0;
    constexpr std::size_t TaskCount = 10'000;
    for (std::size_t i = 0; i < TaskCount; ++i) {
        executor->Post([&completed] {
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    executor->Stop();
    EXPECT_EQ(completed.load(std::memory_order_relaxed), TaskCount);
}

TEST(TTbbExecutorTest, AutomaticConcurrencyDrainsPostedTasksOnDestruction) {
    std::atomic<std::size_t> completed = 0;
    constexpr std::size_t TaskCount = 10'000;
    {
        auto executor = CreateThreadPoolExecutor(0);
        executor->Start();
        for (std::size_t i = 0; i < TaskCount; ++i) {
            executor->Post([&completed] {
                completed.fetch_add(1, std::memory_order_relaxed);
            });
        }
    }

    EXPECT_EQ(completed.load(std::memory_order_relaxed), TaskCount);
}

TEST(TTbbExecutorTest, PostedTaskRunsWithoutWaitingForExecutorShutdown) {
    auto executor = CreateThreadPoolExecutor(1);
    executor->Start();

    std::promise<void> completed;
    auto future = completed.get_future();
    executor->Post([&completed] {
        completed.set_value();
    });

    EXPECT_EQ(future.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    executor->Stop();
}

TEST(TTbbExecutorTest, AutomaticConcurrencyKeepsPostingUnbounded) {
    auto executor = CreateThreadPoolExecutor(0, 1);
    executor->Start();

    std::mutex mutex;
    std::condition_variable released;
    bool canFinish = false;
    constexpr std::size_t TaskCount = 1'000;
    auto posting = std::async(std::launch::async, [&] {
        for (std::size_t i = 0; i < TaskCount; ++i) {
            executor->Post([&] {
                std::unique_lock guard(mutex);
                released.wait(guard, [&] {
                    return canFinish;
                });
            });
        }
    });

    const auto status = posting.wait_for(std::chrono::seconds(5));
    {
        std::lock_guard guard(mutex);
        canFinish = true;
    }
    released.notify_all();

    EXPECT_EQ(status, std::future_status::ready);
    posting.wait();
    executor->Stop();
}

} // namespace
} // namespace NYdb::inline V3
