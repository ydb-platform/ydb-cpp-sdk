#include "topic.h"

#include <tests/slo_workloads/utils/metrics.h>

#include <library/cpp/getopt/last_getopt.h>

#include <util/stream/output.h>
#include <util/string/builder.h>
#include <util/string/cast.h>
#include <util/system/env.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

using namespace NLastGetopt;
using namespace NYdb;
using namespace NYdb::NTopic;

namespace {

// TopicReaderOptions in the Rust workload defaults to 1,000 messages.  The
// native C++ SDK exposes server/decompression events instead, so RunReader
// combines events from one partition-session epoch up to the same limit.
constexpr std::size_t kTopicReadBatchSize = 1'000;

TStatus SuccessStatus() {
    return TStatus(EStatus::SUCCESS, NIssue::TIssues());
}

TStatus ErrorStatus() {
    return TStatus(EStatus::CLIENT_INTERNAL_ERROR, NIssue::TIssues());
}

std::string SanitizePathPart(std::string value) {
    for (char& c : value) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_') {
            c = '-';
        }
    }
    return value;
}

std::string MakeTopicPath(const TDatabaseOptions& dbOptions) {
    std::string workload = GetEnv("WORKLOAD_NAME");
    std::string ref = GetEnv("WORKLOAD_REF");
    if (workload.empty()) {
        workload = "cpp-topic";
    }
    if (ref.empty()) {
        ref = "current";
    }
    return JoinPath(
        dbOptions.Prefix,
        SanitizePathPart(workload) + "-" + SanitizePathPart(ref) + "-topic");
}

std::string PartitionSessionKey(const TPartitionSession::TPtr& partitionSession) {
    return TStringBuilder()
        << partitionSession->GetReadSessionId() << ':'
        << partitionSession->GetPartitionSessionId();
}

std::optional<std::string> MessageError(
    const TReadSessionEvent::TDataReceivedEvent::TMessage& message,
    std::unordered_map<std::string, std::uint64_t>& producerSeqNos,
    std::unordered_map<std::string, std::uint64_t>& streamSeqNos,
    const std::unordered_map<std::uint64_t, std::uint64_t>& committedOffsets)
{
    const std::uint64_t seqNo = message.GetSeqNo();
    const std::string expected = ToString(seqNo);
    if (message.GetData() != expected) {
        return TStringBuilder() << "payload mismatch for seq_no " << seqNo;
    }

    const std::string& producerId = message.GetProducerId();
    auto producer = producerSeqNos.find(producerId);
    if (producer == producerSeqNos.end()) {
        if (seqNo != 1) {
            return TStringBuilder() << "producer " << producerId
                << " starts at seq_no " << seqNo;
        }
        producerSeqNos.emplace(producerId, seqNo);
    } else if (seqNo == producer->second + 1) {
        producer->second = seqNo;
    } else if (seqNo > producer->second) {
        return TStringBuilder() << "producer " << producerId
            << " sequence gap after " << producer->second << ", got " << seqNo;
    }

    const auto partition = message.GetPartitionSession();
    const std::string streamKey = TStringBuilder()
        << producerId << ':' << PartitionSessionKey(partition);
    auto stream = streamSeqNos.find(streamKey);
    if (stream == streamSeqNos.end()) {
        streamSeqNos.emplace(streamKey, seqNo);
    } else if (seqNo == stream->second + 1) {
        stream->second = seqNo;
    } else {
        return TStringBuilder() << "stream sequence after " << stream->second
            << ", got " << seqNo;
    }

    const std::uint64_t partitionId = partition->GetPartitionId();
    if (auto it = committedOffsets.find(partitionId);
        it != committedOffsets.end() && message.GetOffset() < it->second)
    {
        return TStringBuilder() << "partition " << partitionId << " offset "
            << message.GetOffset() << " is below committed offset " << it->second;
    }

    return std::nullopt;
}

} // namespace

struct TTopicRunContext::TImpl {
    explicit TImpl(const TTopicOptions& options)
        : Options(options)
        , Deadline(TInstant::Now() + TDuration::Seconds(options.SecondsToRun))
        , ReadStats(
            options.DontPushMetrics ? std::nullopt : std::make_optional(options.MetricsPushUrl),
            "read")
        , WriteStats(
            options.DontPushMetrics ? std::nullopt : std::make_optional(options.MetricsPushUrl),
            "write")
        , E2eMetrics(options.DontPushMetrics
            ? CreateNoopMetricsPusher()
            : CreateOtelMetricsPusher(options.MetricsPushUrl, "topic_e2e"))
    {}

    void ActivatePartitionSession(std::uint64_t partitionId, const std::string& sessionKey) {
        std::lock_guard lock(Mutex);
        ActivePartitionSessions[partitionId] = sessionKey;
    }

    void RetirePartitionSession(std::uint64_t partitionId, const std::string& sessionKey) {
        std::lock_guard lock(Mutex);
        if (auto it = ActivePartitionSessions.find(partitionId);
            it != ActivePartitionSessions.end() && it->second == sessionKey)
        {
            ActivePartitionSessions.erase(it);
        }
    }

    bool IsActivePartitionSession(
        std::uint64_t partitionId,
        const std::string& sessionKey) const
    {
        std::lock_guard lock(Mutex);
        const auto it = ActivePartitionSessions.find(partitionId);
        return it != ActivePartitionSessions.end() && it->second == sessionKey;
    }

    bool RecordCommittedEndOffset(
        std::uint64_t partitionId,
        const std::string& sessionKey,
        std::uint64_t endOffset)
    {
        std::lock_guard lock(Mutex);
        const auto active = ActivePartitionSessions.find(partitionId);
        if (active == ActivePartitionSessions.end() || active->second != sessionKey) {
            return false;
        }
        auto& current = CommittedOffsets[partitionId];
        current = std::max(current, endOffset);
        return true;
    }

    TTopicOptions Options;
    TInstant Deadline;
    TStat ReadStats;
    TStat WriteStats;
    std::unique_ptr<IMetricsPusher> E2eMetrics;

    std::atomic<bool> Failure{false};
    mutable std::mutex FailureMutex;
    std::string FailureMessage;

    mutable std::mutex Mutex;
    std::unordered_map<std::string, std::uint64_t> ProducerSeqNos;
    std::unordered_map<std::string, std::uint64_t> StreamSeqNos;
    std::unordered_map<std::uint64_t, std::uint64_t> CommittedOffsets;
    std::unordered_map<std::uint64_t, std::string> ActivePartitionSessions;
};

TTopicRunContext::TTopicRunContext(const TTopicOptions& options)
    : Impl_(std::make_unique<TImpl>(options))
{}

TTopicRunContext::~TTopicRunContext() = default;

const TTopicOptions& TTopicRunContext::GetOptions() const {
    return Impl_->Options;
}

bool TTopicRunContext::ShouldContinue() const {
    return !Impl_->Failure.load() && TInstant::Now() < Impl_->Deadline;
}

void TTopicRunContext::Fail(std::string message) {
    if (!Impl_->Failure.exchange(true)) {
        std::lock_guard lock(Impl_->FailureMutex);
        Impl_->FailureMessage = std::move(message);
        Cerr << "Topic workload failed: " << Impl_->FailureMessage << Endl;
    }
}

bool TTopicRunContext::Failed() const {
    return Impl_->Failure.load();
}

void TTopicRunContext::Start() {
    Impl_->ReadStats.Start();
    Impl_->WriteStats.Start();
}

void TTopicRunContext::Finish() {
    Impl_->ReadStats.Finish();
    Impl_->WriteStats.Finish();

    TStringBuilder report;
    report << Endl << "======- Topic read report -======" << Endl;
    Impl_->ReadStats.PrintStatistics(report);
    report << Endl << "======- Topic write report -=====" << Endl;
    Impl_->WriteStats.PrintStatistics(report);
    Cout << report;
}

std::shared_ptr<TStatUnit> TTopicRunContext::StartWrite() {
    return Impl_->WriteStats.StartRequest();
}

void TTopicRunContext::FinishWrite(
    const std::shared_ptr<TStatUnit>& stat,
    const TFinalStatus& status)
{
    Impl_->WriteStats.FinishRequest(stat, status);
}

void TTopicRunContext::CancelWrite(const std::shared_ptr<TStatUnit>& stat) {
    Impl_->WriteStats.CancelRequest(stat);
}

TTopicRateLimiter::TTopicRateLimiter(std::uint32_t rps)
    : Interval_(std::max<std::uint64_t>(1, 1'000'000 / std::max<std::uint32_t>(1, rps)))
    , Next_(TClock::now())
{}

void TTopicRateLimiter::Wait(const TTopicSleep& sleep) {
    TClock::time_point scheduled;
    {
        std::lock_guard lock(Mutex_);
        const auto now = TClock::now();
        scheduled = std::max(Next_, now);
        Next_ = scheduled + Interval_;
    }

    const auto now = TClock::now();
    if (now < scheduled) {
        const auto delay = std::chrono::ceil<std::chrono::microseconds>(scheduled - now);
        sleep(TDuration::MicroSeconds(delay.count()));
    }
}

void TTopicRunContext::RunReader(std::unique_ptr<ITopicReadSession> session) {
    using TReadEvent = TReadSessionEvent::TEvent;

    std::deque<TReadEvent> queuedEvents;
    std::unordered_set<std::string> retiredPartitionSessions;
    auto waitEvents = [&](TDuration timeout) {
        if (!queuedEvents.empty()) {
            return ETopicWaitResult::Events;
        }

        std::vector<TReadEvent> events;
        const auto result = session->WaitEvents(timeout, events);
        if (result == ETopicWaitResult::Events) {
            for (auto& event : events) {
                queuedEvents.push_back(std::move(event));
            }
        }
        return result;
    };

    auto restoreDeferredEvents = [&](std::deque<TReadEvent>& deferredEvents) {
        while (!queuedEvents.empty()) {
            deferredEvents.push_back(std::move(queuedEvents.front()));
            queuedEvents.pop_front();
        }
        queuedEvents.swap(deferredEvents);
    };

    auto handleControlEvent = [&](TReadEvent& event) -> std::optional<std::string> {
        if (std::holds_alternative<TReadSessionEvent::TCommitOffsetAcknowledgementEvent>(event)) {
            // Acknowledgements are correlated with the pending batch below.
            // In particular, do not record the acknowledgement's potentially
            // coalesced offset: Rust records the submitted batch end offset.
        } else if (auto* start = std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(&event)) {
            const auto partitionSession = start->GetPartitionSession();
            const std::string sessionKey = PartitionSessionKey(partitionSession);
            retiredPartitionSessions.erase(sessionKey);
            Impl_->ActivatePartitionSession(partitionSession->GetPartitionId(), sessionKey);
            start->Confirm();
        } else if (auto* stop = std::get_if<TReadSessionEvent::TStopPartitionSessionEvent>(&event)) {
            const auto partitionSession = stop->GetPartitionSession();
            const std::string sessionKey = PartitionSessionKey(partitionSession);
            retiredPartitionSessions.insert(sessionKey);
            Impl_->RetirePartitionSession(partitionSession->GetPartitionId(), sessionKey);
            stop->Confirm();
        } else if (auto* end = std::get_if<TReadSessionEvent::TEndPartitionSessionEvent>(&event)) {
            // Rust closes an ended partition session for new input, but keeps
            // its buffered messages available until they have been drained.
            // A stop/closed event, in contrast, discards that buffered input.
            end->Confirm();
        } else if (auto* closed =
                       std::get_if<TReadSessionEvent::TPartitionSessionClosedEvent>(&event))
        {
            const auto partitionSession = closed->GetPartitionSession();
            const std::string sessionKey = PartitionSessionKey(partitionSession);
            retiredPartitionSessions.insert(sessionKey);
            Impl_->RetirePartitionSession(partitionSession->GetPartitionId(), sessionKey);
        } else if (auto* closed = std::get_if<TSessionClosedEvent>(&event)) {
            if (!closed->IsSuccess()) {
                return TStringBuilder()
                    << "read session closed: " << closed->GetIssues().ToString();
            }
            if (ShouldContinue()) {
                return "read session closed unexpectedly";
            }
        }
        return std::nullopt;
    };

    while (ShouldContinue()) {
        auto delivery = Impl_->ReadStats.StartRequest();
        ETopicWaitResult waitResult;
        try {
            waitResult = waitEvents(Impl_->Options.DeliveryTimeout);
        } catch (const std::exception& e) {
            Impl_->ReadStats.FinishRequest(delivery, ErrorStatus());
            Fail(TStringBuilder() << "reader wait failed: " << e.what());
            break;
        }

        if (waitResult == ETopicWaitResult::Timeout) {
            if (ShouldContinue()) {
                Impl_->ReadStats.FinishRequest(delivery, TFinalStatus{});
            } else {
                Impl_->ReadStats.CancelRequest(delivery);
            }
            continue;
        }
        if (waitResult == ETopicWaitResult::Cancelled) {
            Impl_->ReadStats.CancelRequest(delivery);
            if (ShouldContinue()) {
                Fail("reader wait was cancelled");
            }
            break;
        }
        if (queuedEvents.empty()) {
            Impl_->ReadStats.CancelRequest(delivery);
            continue;
        }

        auto event = std::move(queuedEvents.front());
        queuedEvents.pop_front();
        if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&event)) {
            const auto partitionSession = data->GetPartitionSession();
            const std::string partitionSessionKey = PartitionSessionKey(partitionSession);
            const std::uint64_t partitionId = partitionSession->GetPartitionId();

            // Rust handles lifecycle messages in its background reader runtime,
            // independently of read_batch/commit_with_ack.  Do the same before
            // exposing buffered data: a later Stop must discard earlier queued
            // data, and a newer Start may transfer the partition to another
            // reader while this reader still has old decompressed events.
            std::optional<std::string> queuedControlError;
            for (auto it = queuedEvents.begin(); it != queuedEvents.end();) {
                if (std::holds_alternative<TReadSessionEvent::TDataReceivedEvent>(*it)) {
                    ++it;
                    continue;
                }
                if (auto controlError = handleControlEvent(*it)) {
                    queuedControlError = std::move(controlError);
                }
                it = queuedEvents.erase(it);
            }
            if (queuedControlError) {
                Impl_->ReadStats.FinishRequest(delivery, ErrorStatus());
                Fail(*queuedControlError);
                break;
            }

            if (retiredPartitionSessions.contains(partitionSessionKey) ||
                !Impl_->IsActivePartitionSession(partitionId, partitionSessionKey))
            {
                // The Rust reader runtime drops buffered data from a retired
                // partition-session epoch.  Ownership is global because a new
                // reader can commit ahead before the old reader observes Stop.
                Impl_->ReadStats.CancelRequest(delivery);
                continue;
            }

            std::vector<TReadSessionEvent::TDataReceivedEvent> dataEvents;
            std::size_t messageCount = data->GetMessagesCount();
            dataEvents.emplace_back(std::move(*data));

            // C++ events are grouped by consecutive raw responses.  Traffic
            // from ten partitions interleaves those responses, so committing
            // each event serially limited the harness to a few hundred
            // messages/s and produced the observed 45-60 second queueing delay.
            // Rust instead buffers per partition and pops up to 1,000 messages.
            for (auto it = queuedEvents.begin(); it != queuedEvents.end();) {
                auto* candidate =
                    std::get_if<TReadSessionEvent::TDataReceivedEvent>(&*it);
                if (!candidate ||
                    PartitionSessionKey(candidate->GetPartitionSession()) != partitionSessionKey)
                {
                    ++it;
                    continue;
                }

                const std::size_t candidateCount = candidate->GetMessagesCount();
                if (messageCount + candidateCount > kTopicReadBatchSize) {
                    break;
                }
                messageCount += candidateCount;
                dataEvents.emplace_back(std::move(*candidate));
                it = queuedEvents.erase(it);
            }

            std::uint64_t endOffset = 0;
            std::optional<std::string> error;

            try {
                {
                    std::lock_guard lock(Impl_->Mutex);
                    const auto active = Impl_->ActivePartitionSessions.find(partitionId);
                    if (active == Impl_->ActivePartitionSessions.end() ||
                        active->second != partitionSessionKey)
                    {
                        // A different reader accepted a newer epoch while this
                        // batch was being assembled.
                        Impl_->ReadStats.CancelRequest(delivery);
                        continue;
                    }

                    for (const auto& dataEvent : dataEvents) {
                        for (const auto& message : dataEvent.GetMessages()) {
                            error = MessageError(
                                message,
                                Impl_->ProducerSeqNos,
                                Impl_->StreamSeqNos,
                                Impl_->CommittedOffsets);
                            if (error) {
                                break;
                            }
                            endOffset = std::max(endOffset, message.GetOffset() + 1);
                        }
                        if (error) {
                            break;
                        }
                    }
                }

                if (!error) {
                    for (const auto& dataEvent : dataEvents) {
                        for (const auto& message : dataEvent.GetMessages()) {
                            const TInstant createdAt = message.GetCreateTime();
                            const TInstant now = TInstant::Now();
                            if (createdAt == TInstant::Zero()) {
                                error = "message has no timestamp";
                                break;
                            }
                            if (createdAt > now) {
                                error = TStringBuilder() << "message timestamp is in the future";
                                break;
                            }
                            Impl_->E2eMetrics->PushRequestData({
                                .Delay = now - createdAt,
                                .Status = EStatus::SUCCESS,
                                .RetryAttempts = 0,
                            });
                        }
                        if (error) {
                            break;
                        }
                    }
                }
            } catch (const std::exception& e) {
                error = TStringBuilder() << "message processing failed: " << e.what();
            }

            if (error) {
                Impl_->ReadStats.FinishRequest(delivery, ErrorStatus());
                Fail(*error);
                break;
            }

            // Rust's read_batch operation is cancelled after validation; the
            // successful read metric is the following commit acknowledgement.
            Impl_->ReadStats.CancelRequest(delivery);

            if (!Impl_->IsActivePartitionSession(partitionId, partitionSessionKey)) {
                continue;
            }

            auto commit = Impl_->ReadStats.StartRequest();
            try {
                TDeferredCommit deferredCommit;
                for (const auto& dataEvent : dataEvents) {
                    deferredCommit.Add(dataEvent);
                }
                deferredCommit.Commit();
            } catch (const std::exception& e) {
                Impl_->ReadStats.FinishRequest(commit, ErrorStatus());
                Cerr << "Commit failed: " << e.what() << Endl;
                continue;
            }

            const TInstant commitDeadline = TInstant::Now() + Impl_->Options.CommitTimeout;
            std::deque<TReadEvent> deferredEvents;
            bool commitFinished = false;
            bool stopReader = false;

            while (!commitFinished && !stopReader && ShouldContinue()) {
                const TInstant now = TInstant::Now();
                if (now >= commitDeadline) {
                    Impl_->ReadStats.FinishRequest(commit, TFinalStatus{});
                    commitFinished = true;
                    break;
                }

                try {
                    waitResult = waitEvents(commitDeadline - now);
                } catch (const std::exception& e) {
                    Impl_->ReadStats.FinishRequest(commit, ErrorStatus());
                    Fail(TStringBuilder() << "commit acknowledgement wait failed: " << e.what());
                    commitFinished = true;
                    stopReader = true;
                    break;
                }

                if (waitResult == ETopicWaitResult::Timeout) {
                    if (ShouldContinue()) {
                        Impl_->ReadStats.FinishRequest(commit, TFinalStatus{});
                    } else {
                        Impl_->ReadStats.CancelRequest(commit);
                    }
                    commitFinished = true;
                    break;
                }
                if (waitResult == ETopicWaitResult::Cancelled) {
                    Impl_->ReadStats.CancelRequest(commit);
                    if (ShouldContinue()) {
                        Fail("commit acknowledgement wait was cancelled");
                    }
                    commitFinished = true;
                    stopReader = true;
                    break;
                }

                while (!queuedEvents.empty() && !commitFinished && !stopReader) {
                    auto nextEvent = std::move(queuedEvents.front());
                    queuedEvents.pop_front();
                    auto* ack = std::get_if<
                        TReadSessionEvent::TCommitOffsetAcknowledgementEvent>(&nextEvent);
                    if (!ack) {
                        if (std::holds_alternative<TReadSessionEvent::TDataReceivedEvent>(
                                nextEvent))
                        {
                            deferredEvents.push_back(std::move(nextEvent));
                            continue;
                        }

                        // Match Rust's reader runtime: lifecycle events are
                        // consumed independently while commit_with_ack awaits
                        // its result. In particular, Stop immediately retires
                        // the partition session and discards its buffered data.
                        bool pendingCommitLost = false;
                        if (auto* stop =
                                std::get_if<TReadSessionEvent::TStopPartitionSessionEvent>(
                                    &nextEvent))
                        {
                            pendingCommitLost =
                                PartitionSessionKey(stop->GetPartitionSession()) ==
                                partitionSessionKey;
                        } else if (auto* closed =
                                       std::get_if<
                                           TReadSessionEvent::TPartitionSessionClosedEvent>(
                                           &nextEvent))
                        {
                            pendingCommitLost =
                                PartitionSessionKey(closed->GetPartitionSession()) ==
                                partitionSessionKey;
                        }

                        if (auto controlError = handleControlEvent(nextEvent)) {
                            Impl_->ReadStats.FinishRequest(commit, ErrorStatus());
                            Fail(*controlError);
                            commitFinished = true;
                            stopReader = true;
                        } else if (pendingCommitLost) {
                            // Rust resolves an outstanding commit_with_ack with
                            // an error when its partition session is stopped.
                            Impl_->ReadStats.FinishRequest(commit, ErrorStatus());
                            commitFinished = true;
                        }
                        continue;
                    }

                    const bool isExpectedAck =
                        ack->GetPartitionSession()->GetPartitionId() == partitionId &&
                        PartitionSessionKey(ack->GetPartitionSession()) == partitionSessionKey &&
                        ack->GetCommittedOffset() >= endOffset;

                    if (auto controlError = handleControlEvent(nextEvent)) {
                        Impl_->ReadStats.FinishRequest(commit, ErrorStatus());
                        Fail(*controlError);
                        commitFinished = true;
                        stopReader = true;
                    } else if (isExpectedAck) {
                        // Match Rust's OffsetOrder::insert(partition_id,
                        // batch.end_offset), not the SDK's coalesced ACK value.
                        if (Impl_->RecordCommittedEndOffset(
                                partitionId, partitionSessionKey, endOffset))
                        {
                            Impl_->ReadStats.FinishRequest(commit, SuccessStatus());
                        } else {
                            // A newer epoch owns the partition.  Rust drops the
                            // stopped epoch and fails its pending commit without
                            // publishing that offset to the workload verifier.
                            Impl_->ReadStats.FinishRequest(commit, ErrorStatus());
                        }
                        commitFinished = true;
                    }
                }
            }

            if (!commitFinished) {
                Impl_->ReadStats.CancelRequest(commit);
            }
            restoreDeferredEvents(deferredEvents);
            if (stopReader) {
                break;
            }
        } else if (auto controlError = handleControlEvent(event)) {
            Impl_->ReadStats.FinishRequest(delivery, ErrorStatus());
            Fail(*controlError);
            break;
        } else {
            Impl_->ReadStats.CancelRequest(delivery);
        }
    }

    try {
        session->Close(TDuration::Seconds(5));
    } catch (const std::exception& e) {
        Cerr << "Reader close failed: " << e.what() << Endl;
    }
}

bool ParseTopicOptions(int argc, char** argv, TTopicOptions& options) {
    if (std::string duration = GetEnv("WORKLOAD_DURATION"); !duration.empty()) {
        try {
            options.SecondsToRun = FromString<std::uint32_t>(duration);
        } catch (const std::exception& e) {
            Cerr << "Invalid WORKLOAD_DURATION '" << duration << "': " << e.what() << Endl;
            return false;
        }
    }
    if (std::string url = GetEnv("OTEL_EXPORTER_OTLP_METRICS_ENDPOINT"); !url.empty()) {
        options.MetricsPushUrl = std::move(url);
    }

    std::uint64_t writeTimeoutMs = options.WriteTimeout.MilliSeconds();
    std::uint64_t deliveryTimeoutMs = options.DeliveryTimeout.MilliSeconds();
    std::uint64_t commitTimeoutMs = options.CommitTimeout.MilliSeconds();

    TOpts opts = TOpts::Default();
    opts.AddHelpOption('h');
    opts.AddLongOption("time", "Time to run (seconds)").RequiredArgument("SECONDS")
        .DefaultValue(options.SecondsToRun).StoreResult(&options.SecondsToRun);
    opts.AddLongOption("write-rps", "Total messages submitted per second").RequiredArgument("NUM")
        .DefaultValue(options.WriteRps).StoreResult(&options.WriteRps);
    opts.AddLongOption("write-timeout", "Write acknowledgement timeout (ms)").RequiredArgument("MS")
        .DefaultValue(writeTimeoutMs).StoreResult(&writeTimeoutMs);
    opts.AddLongOption("delivery-timeout", "Message delivery timeout (ms)").RequiredArgument("MS")
        .DefaultValue(deliveryTimeoutMs).StoreResult(&deliveryTimeoutMs);
    opts.AddLongOption("commit-timeout", "Commit acknowledgement timeout (ms)").RequiredArgument("MS")
        .DefaultValue(commitTimeoutMs).StoreResult(&commitTimeoutMs);
    opts.AddLongOption("partition-count", "Topic partition count").RequiredArgument("NUM")
        .DefaultValue(options.PartitionCount).StoreResult(&options.PartitionCount);
    opts.AddLongOption("consumer", "Topic consumer name").RequiredArgument("NAME")
        .DefaultValue(options.ConsumerName).StoreResult(&options.ConsumerName);
    opts.AddLongOption("reader-count", "Topic reader count").RequiredArgument("NUM")
        .DefaultValue(options.ReaderCount).StoreResult(&options.ReaderCount);
    opts.AddLongOption("producer-id-prefix", "Producer id prefix").RequiredArgument("PREFIX")
        .DefaultValue(options.ProducerIdPrefix).StoreResult(&options.ProducerIdPrefix);
    opts.AddLongOption("writer-count", "Topic writer count").RequiredArgument("NUM")
        .DefaultValue(options.WriterCount).StoreResult(&options.WriterCount);
    opts.AddLongOption("dont-push", "Do not push metrics").NoArgument()
        .SetFlag(&options.DontPushMetrics).DefaultValue(options.DontPushMetrics);
    opts.AddLongOption("metrics-push-url", "OTLP metrics endpoint").RequiredArgument("URL")
        .DefaultValue(options.MetricsPushUrl).StoreResult(&options.MetricsPushUrl);
    opts.MutuallyExclusive("dont-push", "metrics-push-url");
    opts.SetFreeArgsNum(0);

    TOptsParseResult result(&opts, argc, argv);
    Y_UNUSED(result);

    options.WriteTimeout = TDuration::MilliSeconds(writeTimeoutMs);
    options.DeliveryTimeout = TDuration::MilliSeconds(deliveryTimeoutMs);
    options.CommitTimeout = TDuration::MilliSeconds(commitTimeoutMs);
    options.TopicPath = MakeTopicPath(options.DatabaseOptions);

    if (!options.SecondsToRun || !options.WriteRps || !options.PartitionCount ||
        !options.ReaderCount || !options.WriterCount || !writeTimeoutMs ||
        !deliveryTimeoutMs || !commitTimeoutMs)
    {
        Cerr << "Topic workload counts, RPS, duration, and timeouts must be greater than zero" << Endl;
        return false;
    }
    return true;
}

TReadSessionSettings MakeTopicReadSettings(const TTopicOptions& options) {
    return TReadSessionSettings()
        .ConsumerName(options.ConsumerName)
        .AppendTopics(options.TopicPath);
}

bool HandleTopicWriteEvent(
    TWriteSessionEvent::TEvent& event,
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

    if (auto* closed = std::get_if<TSessionClosedEvent>(&event)) {
        context.Fail(TStringBuilder()
            << "write session closed: " << closed->GetIssues().ToString());
    } else {
        context.Fail("write session returned an unknown event");
    }
    return false;
}

void RunTopicWriter(
    TTopicClient& client,
    std::uint32_t writerIndex,
    TTopicRunContext& context,
    TTopicRateLimiter& limiter,
    std::atomic<std::uint32_t>& readyWriters,
    const TTopicSleep& sleep)
{
    const auto& options = context.GetOptions();
    const std::string producerId = options.ProducerIdPrefix + "-" + ToString(writerIndex);
    TWriteSessionSettings settings;
    settings.Path(options.TopicPath)
        .ProducerId(producerId)
        .MessageGroupId(producerId)
        .Codec(ECodec::RAW);
    auto session = client.CreateWriteSession(settings);
    std::uint64_t seqNo = 1;
    std::optional<TContinuationToken> token;

    // Rust awaits TopicWriter::new before it starts write spans. Do the same
    // here so initial session establishment is not reported as a timed-out
    // write operation.
    try {
        while (!token && context.ShouldContinue()) {
            if (!session->WaitEvent().Wait(options.WriteTimeout)) {
                continue;
            }
            bool unusedAck = false;
            for (auto& event : session->GetEvents(false)) {
                if (!HandleTopicWriteEvent(
                        event, token, std::nullopt, unusedAck, context))
                {
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        context.Fail(TStringBuilder()
            << "write session initialization failed: " << e.what());
    }

    if (token && context.ShouldContinue()) {
        readyWriters.fetch_add(1);
        while (readyWriters.load() < options.WriterCount && context.ShouldContinue()) {
            sleep(TDuration::MilliSeconds(1));
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
        const TInstant operationDeadline = TInstant::Now() + options.WriteTimeout;
        try {
            bool unusedAck = false;
            while (!token && context.ShouldContinue()) {
                const TInstant now = TInstant::Now();
                if (now >= operationDeadline ||
                    !session->WaitEvent().Wait(operationDeadline - now))
                {
                    break;
                }
                for (auto& event : session->GetEvents(false)) {
                    if (!HandleTopicWriteEvent(
                            event, token, std::nullopt, unusedAck, context))
                    {
                        break;
                    }
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
                const TInstant now = TInstant::Now();
                if (now >= operationDeadline ||
                    !session->WaitEvent().Wait(operationDeadline - now))
                {
                    break;
                }
                for (auto& event : session->GetEvents(false)) {
                    if (!HandleTopicWriteEvent(
                            event, token, submittedSeqNo, acked, context))
                    {
                        break;
                    }
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
        } catch (const std::exception& e) {
            context.FinishWrite(stat, ErrorStatus());
            context.Fail(TStringBuilder() << "write failed: " << e.what());
            break;
        }
    }

    if (!session->Close(options.WriteTimeout) && !context.Failed()) {
        context.Fail("topic write session close timed out");
    }
}

int DoCreate(TDatabaseOptions& dbOptions, int argc, char** argv) {
    TTopicOptions options(dbOptions);
    if (!ParseTopicOptions(argc, argv, options)) {
        return EXIT_FAILURE;
    }

    TCreateTopicSettings settings;
    settings.PartitioningSettings(options.PartitionCount, options.PartitionCount);
    settings.BeginAddConsumer(options.ConsumerName).Important(true);

    TTopicClient client(dbOptions.Driver);
    const TStatus status = client.CreateTopic(options.TopicPath, settings).GetValueSync();
    if (!status.IsSuccess()) {
        Cerr << "CreateTopic failed: " << status << Endl;
        return EXIT_FAILURE;
    }
    Cout << "Topic created: " << options.TopicPath << Endl;
    return EXIT_SUCCESS;
}

int DoCleanup(TDatabaseOptions& dbOptions, int argc) {
    if (argc > 1) {
        Cerr << "Unexpected arguments after cleanup" << Endl;
        return EXIT_FAILURE;
    }

    TTopicOptions options(dbOptions);
    options.TopicPath = MakeTopicPath(dbOptions);
    TTopicClient client(dbOptions.Driver);
    TStatus status = client.DropTopic(options.TopicPath).GetValueSync();
    if (status.GetStatus() == EStatus::NOT_FOUND) {
        return EXIT_SUCCESS;
    }
    if (!status.IsSuccess()) {
        Cerr << "DropTopic failed: " << status << Endl;
        return EXIT_FAILURE;
    }
    Cout << "Topic dropped: " << options.TopicPath << Endl;
    return EXIT_SUCCESS;
}
