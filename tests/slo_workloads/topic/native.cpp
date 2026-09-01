#include "topic.h"

#include <util/system/thread.h>

#include <thread>
#include <vector>

using namespace NYdb::NTopic;

namespace {

class TNativeTopicReadSession final : public ITopicReadSession {
public:
    explicit TNativeTopicReadSession(std::shared_ptr<IReadSession> session)
        : Session_(std::move(session))
    {}

    ETopicWaitResult WaitEvents(
        TDuration timeout,
        std::vector<TReadSessionEvent::TEvent>& events) override
    {
        if (!Session_->WaitEvent().Wait(timeout)) {
            return ETopicWaitResult::Timeout;
        }
        // Let the SDK drain and coalesce every event that is already ready.
        // Limiting this to one event turns interleaved partition traffic into
        // tiny batches while a commit acknowledgement is pending.
        events = Session_->GetEvents(false);
        return ETopicWaitResult::Events;
    }

    void Close(TDuration timeout) override {
        Session_->Close(timeout);
    }

private:
    std::shared_ptr<IReadSession> Session_;
};

} // namespace

int DoRun(TDatabaseOptions& dbOptions, int argc, char** argv) {
    TTopicOptions options(dbOptions);
    if (!ParseTopicOptions(argc, argv, options)) {
        return EXIT_FAILURE;
    }

    TTopicClient client(dbOptions.Driver);
    TTopicRunContext context(options);
    std::vector<std::thread> threads;
    threads.reserve(options.ReaderCount + options.WriterCount);
    TTopicRateLimiter limiter(options.WriteRps);
    std::atomic<std::uint32_t> readyWriters{0};

    context.Start();
    for (std::uint32_t i = 0; i < options.ReaderCount; ++i) {
        auto session = std::make_unique<TNativeTopicReadSession>(
            client.CreateReadSession(MakeTopicReadSettings(options)));
        threads.emplace_back([&context, session = std::move(session)]() mutable {
            context.RunReader(std::move(session));
        });
    }
    for (std::uint32_t i = 0; i < options.WriterCount; ++i) {
        threads.emplace_back([&client, &context, &limiter, &readyWriters, i] {
            RunTopicWriter(client, i, context, limiter, readyWriters, [](TDuration duration) {
                Sleep(duration);
            });
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
    context.Finish();

    return context.Failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
