#include <tests/slo_workloads/topic/topic.h>

#include <tests/slo_workloads/userver/key_value/userver_table_client.h>

#include <userver/drivers/subscribable_futures.hpp>
#include <userver/engine/async.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/engine/future_status.hpp>
#include <userver/engine/get_all.hpp>
#include <userver/ydb/topic.hpp>

#include <util/system/thread.h>

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

using namespace NYdb::NTopic;

namespace {

class TUserverTopicReadSession final : public ITopicReadSession {
public:
    explicit TUserverTopicReadSession(userver::ydb::TopicReadSession session)
        : Session_(std::move(session))
    {}

    ETopicWaitResult WaitEvents(
        TDuration timeout,
        std::vector<TReadSessionEvent::TEvent>& events) override
    {
        if (!Waiter_) {
            Waiter_ = std::make_unique<TWaiter>(
                Session_.GetNativeTopicReadSession()->WaitEvent());
        }

        const auto status = Waiter_->TryWaitUntil(userver::engine::Deadline::FromDuration(
            std::chrono::microseconds(timeout.MicroSeconds())));
        if (status == userver::engine::FutureStatus::kTimeout) {
            return ETopicWaitResult::Timeout;
        }
        if (status == userver::engine::FutureStatus::kCancelled) {
            Waiter_.reset();
            return ETopicWaitResult::Cancelled;
        }

        Waiter_.reset();
        events = Session_.GetEvents();
        return ETopicWaitResult::Events;
    }

    void Close(TDuration timeout) override {
        Session_.Close(std::chrono::milliseconds(timeout.MilliSeconds()));
    }

private:
    using TWaiter = userver::drivers::SubscribableFutureWrapper<NThreading::TFuture<void>>;

    userver::ydb::TopicReadSession Session_;
    std::unique_ptr<TWaiter> Waiter_;
};

} // namespace

int DoRun(TDatabaseOptions& dbOptions, int argc, char** argv) {
    TTopicOptions options(dbOptions);
    if (!ParseTopicOptions(argc, argv, options)) {
        return EXIT_FAILURE;
    }

    auto& userverClient = userver_slo::GetTopicClient();
    auto& nativeClient = userverClient.GetNativeTopicClient();
    TTopicRunContext context(options);
    std::vector<userver::engine::TaskWithResult<void>> tasks;
    std::vector<std::thread> writers;
    tasks.reserve(options.ReaderCount);
    writers.reserve(options.WriterCount);

    context.Start();
    for (std::uint32_t i = 0; i < options.ReaderCount; ++i) {
        auto session = std::make_unique<TUserverTopicReadSession>(
            userverClient.CreateReadSession(MakeTopicReadSettings(options)));
        tasks.push_back(userver::engine::AsyncNoSpan(
            [&context, session = std::move(session)]() mutable {
                context.RunReader(std::move(session));
            }));
    }
    // userver v2.12 wraps topic reads and control-plane calls, but not writes.
    for (std::uint32_t i = 0; i < options.WriterCount; ++i) {
        writers.emplace_back([&nativeClient, &context, i] {
            RunTopicWriter(nativeClient, i, context, [](TDuration duration) {
                Sleep(duration);
            });
        });
    }

    userver::engine::GetAll(tasks);
    for (auto& writer : writers) {
        writer.join();
    }
    context.Finish();
    return context.Failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
