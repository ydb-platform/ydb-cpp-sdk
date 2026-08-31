#include <tests/slo_workloads/topic/topic.h>

#include <tests/slo_workloads/userver/key_value/userver_table_client.h>

#include <userver/concurrent/mpsc_queue.hpp>
#include <userver/engine/async.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/engine/exception.hpp>
#include <userver/engine/get_all.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/ydb/exceptions.hpp>
#include <userver/ydb/topic.hpp>

#include <util/string/builder.h>
#include <util/string/cast.h>

#include <algorithm>
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
        TDuration,
        std::vector<TReadSessionEvent::TEvent>& events) override
    {
        try {
            events = Session_.GetEvents();
        } catch (const userver::ydb::OperationCancelledError&) {
            return ETopicWaitResult::Cancelled;
        }
        return ETopicWaitResult::Events;
    }

    void Close(TDuration timeout) override {
        Session_.Close(std::chrono::milliseconds(timeout.MilliSeconds()));
    }

private:
    userver::ydb::TopicReadSession Session_;
};

using TWriteEvent = TWriteSessionEvent::TEvent;
using TWriteEventQueue = userver::concurrent::MpscQueue<TWriteEvent>;

bool HandleWriteEvent(
    TWriteEvent& event,
    std::optional<TContinuationToken>& token,
    std::optional<std::uint64_t> expectedAck,
    bool& acked,
    TTopicRunContext& context)
{
    if (auto* ready = std::get_if<TWriteSessionEvent::TReadyToAcceptEvent>(&event)) {
        if (token) {
            context.Fail("write session returned an extra continuation token");
            return false;
        }
        token.emplace(std::move(ready->ContinuationToken));
        return true;
    }

    if (auto* acks = std::get_if<TWriteSessionEvent::TAcksEvent>(&event)) {
        for (const auto& ack : acks->Acks) {
            if (!expectedAck || ack.SeqNo < *expectedAck) {
                continue;
            }
            if (ack.SeqNo > *expectedAck) {
                context.Fail(TStringBuilder()
                    << "unexpected write ack for seq_no " << ack.SeqNo
                    << ", expected " << *expectedAck);
                return false;
            }
            if (ack.State != TWriteSessionEvent::TWriteAck::EES_WRITTEN &&
                ack.State != TWriteSessionEvent::TWriteAck::EES_ALREADY_WRITTEN)
            {
                context.Fail(TStringBuilder()
                    << "write was not persisted at seq_no " << ack.SeqNo
                    << ", ack state " << static_cast<int>(ack.State));
                return false;
            }
            acked = true;
        }
        return true;
    }

    const auto& closed = std::get<TSessionClosedEvent>(event);
    context.Fail(TStringBuilder()
        << "write session closed: " << closed.GetIssues().ToString());
    return false;
}

void RunUserverTopicWriter(
    userver::ydb::TopicClient& client,
    std::uint32_t writerIndex,
    TTopicRunContext& context)
{
    const auto& options = context.GetOptions();
    const std::uint32_t workerRps = options.WriteRps / options.WriterCount +
        (writerIndex < options.WriteRps % options.WriterCount ? 1 : 0);
    if (!workerRps) {
        while (context.ShouldContinue()) {
            userver::engine::SleepFor(std::chrono::milliseconds(100));
        }
        return;
    }

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
    auto eventTask = userver::engine::AsyncNoSpan(
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

    const TDuration period = TDuration::MicroSeconds(1'000'000 / workerRps);
    TInstant next = TInstant::Now();
    std::uint64_t seqNo = 1;
    std::optional<TContinuationToken> token;

    while (context.ShouldContinue()) {
        const TInstant now = TInstant::Now();
        if (now < next) {
            userver::engine::SleepFor(std::chrono::microseconds((next - now).MicroSeconds()));
        }
        next = std::max(next + period, TInstant::Now());
        if (!context.ShouldContinue()) {
            break;
        }

        auto stat = context.StartWrite();
        try {
            const auto tokenDeadline = userver::engine::Deadline::FromDuration(
                std::chrono::microseconds(options.WriteTimeout.MicroSeconds()));
            bool unusedAck = false;
            while (!token && context.ShouldContinue()) {
                TWriteEvent event;
                if (!eventConsumer.Pop(event, tokenDeadline)) {
                    break;
                }
                if (!HandleWriteEvent(event, token, std::nullopt, unusedAck, context)) {
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

            TWriteMessage message(ToString(seqNo));
            message.SeqNo(seqNo);
            session->Write(std::move(*token), std::move(message));
            token.reset();

            const auto ackDeadline = userver::engine::Deadline::FromDuration(
                std::chrono::microseconds(options.WriteTimeout.MicroSeconds()));
            bool acked = false;
            while (!acked && context.ShouldContinue()) {
                TWriteEvent event;
                if (!eventConsumer.Pop(event, ackDeadline)) {
                    break;
                }
                if (!HandleWriteEvent(event, token, seqNo, acked, context)) {
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
        ++seqNo;
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

    context.Start();
    for (std::uint32_t i = 0; i < options.ReaderCount; ++i) {
        auto session = std::make_unique<TUserverTopicReadSession>(
            userverClient.CreateReadSession(MakeTopicReadSettings(options)));
        readers.push_back(userver::engine::AsyncNoSpan(
            [&context, session = std::move(session)]() mutable {
                context.RunReader(std::move(session));
            }));
    }
    for (std::uint32_t i = 0; i < options.WriterCount; ++i) {
        writers.push_back(userver::engine::AsyncNoSpan(
            [&userverClient, &context, i] {
                try {
                    RunUserverTopicWriter(userverClient, i, context);
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
