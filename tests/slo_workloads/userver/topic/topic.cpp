#include <tests/slo_workloads/topic/topic.h>

#include <tests/slo_workloads/userver/key_value/userver_table_client.h>

#include <userver/concurrent/mpsc_queue.hpp>
#include <userver/engine/async.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/engine/exception.hpp>
#include <userver/engine/get_all.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/ydb/exceptions.hpp>
#include <userver/ydb/topic.hpp>

#include <util/string/builder.h>
#include <util/string/cast.h>

#include <chrono>
#include <memory>
#include <vector>

using namespace NYdb::NTopic;

namespace {

NYdb::TStatus SuccessStatus() {
    return NYdb::TStatus(NYdb::EStatus::SUCCESS, NYdb::NIssue::TIssues());
}

NYdb::TStatus ErrorStatus() {
    return NYdb::TStatus(NYdb::EStatus::CLIENT_INTERNAL_ERROR, NYdb::NIssue::TIssues());
}

class TUserverTopicReadSession final : public ITopicReadSession {
public:
    explicit TUserverTopicReadSession(userver::ydb::TopicReadSession session)
        : Session_(std::move(session))
    {}

    ETopicWaitResult WaitEvents(
        TDuration timeout,
        std::vector<TReadSessionEvent::TEvent>& events) override
    {
        if (!Waiter_.IsValid()) {
            Waiter_ = userver::engine::AsyncNoTracing([this] {
                // TopicReadSession suspends this engine task and then drains
                // the native SDK queue using its automatic batch size.
                return Session_.GetEvents();
            });
        }

        try {
            Waiter_.WaitFor(std::chrono::microseconds(timeout.MicroSeconds()));
            if (!Waiter_.IsFinished()) {
                return ETopicWaitResult::Timeout;
            }
            events = Waiter_.Get();
        } catch (const userver::engine::WaitInterruptedException&) {
            return ETopicWaitResult::Cancelled;
        } catch (const userver::ydb::OperationCancelledError&) {
            return ETopicWaitResult::Cancelled;
        }
        return ETopicWaitResult::Events;
    }

    void Close(TDuration timeout) override {
        if (Waiter_.IsValid()) {
            Waiter_.SyncCancel();
            try {
                Waiter_.Get();
            } catch (const std::exception&) {
                // The pending GetEvents task is expected to observe cancellation.
            }
        }
        Session_.Close(std::chrono::milliseconds(timeout.MilliSeconds()));
    }

private:
    userver::ydb::TopicReadSession Session_;
    userver::engine::TaskWithResult<std::vector<TReadSessionEvent::TEvent>> Waiter_;
};

using TWriteEvent = TWriteSessionEvent::TEvent;
using TWriteEventQueue = userver::concurrent::MpscQueue<TWriteEvent>;

void RunUserverTopicWriter(
    userver::ydb::TopicClient& client,
    std::uint32_t writerIndex,
    TTopicRunContext& context,
    TTopicRateLimiter& limiter,
    std::atomic<std::uint32_t>& readyWriters)
{
    const auto& options = context.GetOptions();
    const std::string producerId = options.ProducerIdPrefix + "-" + ToString(writerIndex);
    TWriteSessionSettings settings;
    settings.Path(options.TopicPath)
        .ProducerId(producerId)
        .MessageGroupId(producerId)
        .Codec(ECodec::RAW);
    auto session = std::make_shared<userver::ydb::TopicWriteSession>(
        client.CreateWriteSession(settings));

    auto eventQueue = TWriteEventQueue::Create(16);
    auto eventConsumer = eventQueue->GetConsumer();
    auto eventTask = userver::engine::AsyncNoTracing(
        [session, producer = eventQueue->GetProducer(), &context, writerIndex]() mutable {
            try {
                while (true) {
                    auto event = session->GetEvent();
                    const bool closed = std::holds_alternative<TSessionClosedEvent>(event);
                    if (!producer.Push(std::move(event)) || closed) {
                        break;
                    }
                }
            } catch (const userver::ydb::OperationCancelledError&) {
                // Normal shutdown: the writer cancels this event pump.
            } catch (const std::exception& e) {
                context.Fail(TStringBuilder()
                    << "write event pump " << writerIndex << " failed: " << e.what());
            }
        });

    std::uint64_t seqNo = 1;
    std::optional<TContinuationToken> token;
    const TTopicSleep sleep = [](TDuration duration) {
        userver::engine::SleepFor(std::chrono::microseconds(duration.MicroSeconds()));
    };

    // Match Rust's async TopicWriter::new: establish the write session before
    // starting operation spans, rather than charging initial readiness to the
    // first write's timeout.
    try {
        while (!token && context.ShouldContinue()) {
            TWriteEvent event;
            const auto readyDeadline = userver::engine::Deadline::FromDuration(
                std::chrono::microseconds(options.WriteTimeout.MicroSeconds()));
            if (!eventConsumer.Pop(event, readyDeadline)) {
                continue;
            }
            bool unusedAck = false;
            if (!HandleTopicWriteEvent(event, token, std::nullopt, unusedAck, context)) {
                break;
            }
        }
    } catch (const userver::ydb::OperationCancelledError&) {
        if (context.ShouldContinue()) {
            context.Fail(TStringBuilder()
                << "write session initialization was cancelled for writer " << writerIndex);
        }
    } catch (const std::exception& e) {
        context.Fail(TStringBuilder()
            << "write session initialization failed for writer " << writerIndex
            << ": " << e.what());
    }

    if (token && context.ShouldContinue()) {
        readyWriters.fetch_add(1);
        while (readyWriters.load() < options.WriterCount && context.ShouldContinue()) {
            userver::engine::SleepFor(std::chrono::milliseconds(1));
        }
    }

    while (context.ShouldContinue()) {
        limiter.Wait(sleep);
        if (!context.ShouldContinue()) {
            break;
        }

        const std::string payload = ToString(seqNo);
        TWriteMessage message(payload);
        message.SeqNo(seqNo).CreateTimestamp(TInstant::Now());
        auto stat = context.StartWrite();
        try {
            const auto operationDeadline = userver::engine::Deadline::FromDuration(
                std::chrono::microseconds(options.WriteTimeout.MicroSeconds()));
            bool unusedAck = false;
            while (!token && context.ShouldContinue()) {
                TWriteEvent event;
                if (!eventConsumer.Pop(event, operationDeadline)) {
                    break;
                }
                if (!HandleTopicWriteEvent(event, token, std::nullopt, unusedAck, context)) {
                    break;
                }
            }

            if (!token) {
                if (context.Failed()) {
                    context.FinishWrite(stat, ErrorStatus());
                    break;
                }
                if (!context.ShouldContinue()) {
                    context.CancelWrite(stat);
                    break;
                }
                context.FinishWrite(stat, TFinalStatus{});
                continue;
            }

            const std::uint64_t submittedSeqNo = seqNo++;
            session->Write(std::move(*token), std::move(message));
            token.reset();

            bool acked = false;
            while (!acked && context.ShouldContinue()) {
                TWriteEvent event;
                if (!eventConsumer.Pop(event, operationDeadline)) {
                    break;
                }
                if (!HandleTopicWriteEvent(
                        event, token, submittedSeqNo, acked, context))
                {
                    break;
                }
            }

            if (acked) {
                context.FinishWrite(stat, SuccessStatus());
            } else if (context.Failed()) {
                context.FinishWrite(stat, ErrorStatus());
                break;
            } else if (!context.ShouldContinue()) {
                context.CancelWrite(stat);
                break;
            } else {
                context.FinishWrite(stat, TFinalStatus{});
            }
        } catch (const userver::ydb::OperationCancelledError&) {
            context.CancelWrite(stat);
            if (context.ShouldContinue()) {
                context.Fail(TStringBuilder() << "write was cancelled at seq_no " << seqNo);
            }
            break;
        } catch (const std::exception& e) {
            context.FinishWrite(stat, ErrorStatus());
            context.Fail(TStringBuilder() << "write failed: " << e.what());
            break;
        }
    }

    eventTask.RequestCancel();
    try {
        eventTask.Get();
    } catch (const userver::engine::TaskCancelledException&) {
        // The event pump may be cancelled before its callable starts.
    }

    try {
        if (!session->Close(std::chrono::milliseconds(options.WriteTimeout.MilliSeconds())) &&
            !context.Failed())
        {
            context.Fail("topic write session close timed out");
        }
    } catch (const std::exception& e) {
        context.Fail(TStringBuilder() << "topic write session close failed: " << e.what());
    }
}

} // namespace

int DoRun(TDatabaseOptions& dbOptions, int argc, char** argv) {
    TTopicOptions options(dbOptions);
    if (!ParseTopicOptions(argc, argv, options)) {
        return EXIT_FAILURE;
    }

    auto& userverClient = userver_slo::GetTopicClient();
    TTopicRunContext context(options);
    std::vector<userver::engine::TaskWithResult<void>> readers;
    std::vector<userver::engine::TaskWithResult<void>> writers;
    readers.reserve(options.ReaderCount);
    writers.reserve(options.WriterCount);
    TTopicRateLimiter limiter(options.WriteRps);
    std::atomic<std::uint32_t> readyWriters{0};

    context.Start();
    for (std::uint32_t i = 0; i < options.ReaderCount; ++i) {
        auto session = std::make_unique<TUserverTopicReadSession>(
            userverClient.CreateReadSession(MakeTopicReadSettings(options)));
        readers.push_back(userver::engine::AsyncNoTracing(
            [&context, session = std::move(session)]() mutable {
                context.RunReader(std::move(session));
            }));
    }
    for (std::uint32_t i = 0; i < options.WriterCount; ++i) {
        writers.push_back(userver::engine::AsyncNoTracing(
            [&userverClient, &context, &limiter, &readyWriters, i] {
                try {
                    RunUserverTopicWriter(userverClient, i, context, limiter, readyWriters);
                } catch (const std::exception& e) {
                    context.Fail(TStringBuilder()
                        << "topic writer " << i << " failed: " << e.what());
                }
            }));
    }

    userver::engine::GetAll(writers);
    for (auto& reader : readers) {
        reader.RequestCancel();
    }
    userver::engine::GetAll(readers);
    context.Finish();
    return context.Failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
