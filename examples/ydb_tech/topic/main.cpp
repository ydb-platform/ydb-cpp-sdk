#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/proto/accessor.h>
#include <ydb-cpp-sdk/client/query/client.h>
#include <ydb-cpp-sdk/client/topic/client.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <set>
#include <variant>
#include <vector>

using namespace NYdb::NTopic;
using NYdb::NStatusHelpers::ThrowOnError;

namespace {

void Require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::string Environment(const char* name, const char* fallback) {
    const auto* value = std::getenv(name);
    return value ? value : fallback;
}

const std::map<std::string, size_t> Expected = {
    {"This is yet another message.", 1}, {"message", 1},
    {"This is yet another message", 1}, {"message-data", 1},
    {"no_dedup_async", 1}, {"no_dedup_blocking", 1}, {"acknowledged", 1},
    {"codec_async", 1}, {"codec_blocking", 1},
};
NYdb::TDriver* ProducerDriver = nullptr;

size_t Total(const std::map<std::string, size_t>& values) {
    size_t result = 0;
    for (const auto& [key, count] : values) { (void)key; result += count; }
    return result;
}

class SingleMessageSession final : public IWriteSession {
public:
    explicit SingleMessageSession(std::shared_ptr<IWriteSession> inner) : Inner(std::move(inner)) {}
    NThreading::TFuture<void> WaitEvent() override { return Inner->WaitEvent(); }
    std::optional<TWriteSessionEvent::TEvent> GetEvent(bool block = false) override {
        while (auto event = Inner->GetEvent(block)) {
            if (Written && std::get_if<TWriteSessionEvent::TReadyToAcceptEvent>(&*event)) continue;
            return event;
        }
        return std::nullopt;
    }
    std::vector<TWriteSessionEvent::TEvent> GetEvents(bool block = false, std::optional<size_t> count = std::nullopt) override {
        return Inner->GetEvents(block, count);
    }
    NThreading::TFuture<uint64_t> GetInitSeqNo() override { return Inner->GetInitSeqNo(); }
    void Write(TContinuationToken&& token, TWriteMessage&& message, NYdb::TTransactionBase* tx = nullptr) override {
        Inner->Write(std::move(token), std::move(message), tx);
        Written = true;
        Require(Inner->Close(TDuration::Seconds(30)), "Writer did not close after the demonstration message");
    }
    void Write(TContinuationToken&& token, std::string_view data, std::optional<uint64_t> seqNo = std::nullopt,
               std::optional<TInstant> timestamp = std::nullopt) override {
        Inner->Write(std::move(token), data, seqNo, timestamp);
        Written = true;
        Require(Inner->Close(TDuration::Seconds(30)), "Writer did not close after the demonstration message");
    }
    void WriteEncoded(TContinuationToken&& token, TWriteMessage&& message, NYdb::TTransactionBase* tx = nullptr) override {
        Inner->WriteEncoded(std::move(token), std::move(message), tx);
    }
    void WriteEncoded(TContinuationToken&& token, std::string_view data, ECodec codec, uint32_t size,
                      std::optional<uint64_t> seqNo = std::nullopt, std::optional<TInstant> timestamp = std::nullopt) override {
        Inner->WriteEncoded(std::move(token), data, codec, size, seqNo, timestamp);
    }
    NThreading::TFuture<bool> Flush() override { return Inner->Flush(); }
    bool Close(TDuration timeout = TDuration::Max()) override { return Inner->Close(timeout); }
    TWriterCounters::TPtr GetCounters() override { return Inner->GetCounters(); }
private:
    std::shared_ptr<IWriteSession> Inner;
    bool Written = false;
};

class SeqNumbers {
public:
    void insert(uint64_t value) { std::lock_guard lock(Mutex); Values.insert(value); }
    size_t size() const { std::lock_guard lock(Mutex); return Values.size(); }
private:
    mutable std::mutex Mutex;
    std::set<uint64_t> Values;
};

TContinuationToken NextToken(const std::shared_ptr<IWriteSession>& session) {
    while (true) {
        auto event = session->GetEvent(true);
        Require(event.has_value(), "Writer closed before accepting the message");
        if (auto* ready = std::get_if<TWriteSessionEvent::TReadyToAcceptEvent>(&*event)) {
            return std::move(ready->ContinuationToken);
        }
        if (auto* closed = std::get_if<TSessionClosedEvent>(&*event)) {
            ThrowOnError(*closed);
            throw std::runtime_error("Writer closed before accepting the message");
        }
    }
}

void WriteAsync(TTopicClient& topicClient, const std::string& path) {
    // [BEGIN topic_start_writer]
    std::string producerAndGroupID = "group-id";
    auto settings = NYdb::NTopic::TWriteSessionSettings()
        .Path(path)
        .ProducerId(producerAndGroupID)
        .MessageGroupId(producerAndGroupID);

    auto session = topicClient.CreateWriteSession(settings);
    // [END topic_start_writer]
    session = std::make_shared<SingleMessageSession>(session);
    // [BEGIN topic_write]
    // Event loop
    while (true) {
        // Get event
        // May block for a while if write session is busy
        std::optional<NYdb::NTopic::TWriteSessionEvent::TEvent> event = session->GetEvent(/*block=*/true);

        if (auto* readyEvent = std::get_if<NYdb::NTopic::TWriteSessionEvent::TReadyToAcceptEvent>(&*event)) {
            session->Write(std::move(readyEvent->ContinuationToken), "This is yet another message.");

        } else if (auto* ackEvent = std::get_if<NYdb::NTopic::TWriteSessionEvent::TAcksEvent>(&*event)) {
            std::cout << ackEvent->DebugString() << std::endl;

        } else if (auto* closeSessionEvent = std::get_if<NYdb::NTopic::TSessionClosedEvent>(&*event)) {
            break;
        }
    }
    // [END topic_write]
}

void WriteBlocking(TTopicClient& topicClient, const std::string& path) {
    // [BEGIN topic_start_writer_blocking]
    std::string producerAndGroupID = "group-id";
    auto settings = NYdb::NTopic::TWriteSessionSettings()
        .Path(path)
        .ProducerId(producerAndGroupID)
        .MessageGroupId(producerAndGroupID);

    auto session = topicClient.CreateSimpleBlockingWriteSession(settings);
    // [END topic_start_writer_blocking]
    {
    // [BEGIN topic_write_blocking]
    auto messageData = std::string("message");
    NYdb::NTopic::TWriteMessage writeMessage(messageData);
    session->Write(std::move(writeMessage));
    // [END topic_write_blocking]
    }
    {
    // [BEGIN topic_write_metadata_blocking]
    auto messageData = std::string("message-data");
    NYdb::NTopic::TWriteMessage writeMessage(messageData);
    writeMessage.MessageMeta({
        {"meta-key", "meta-value"},
        {"another-key", "value"},
    });
    session->Write(std::move(writeMessage));
    // [END topic_write_metadata_blocking]
    }
    Require(session->Close(TDuration::Seconds(30)), "Blocking writer did not close");
}

void WriteMetadata(TTopicClient& topicClient, const std::string& path) {
    // [BEGIN topic_write_metadata]
    auto settings = NYdb::NTopic::TWriteSessionSettings()
        .Path(path).DeduplicationEnabled(false)
    // set all other settings;
    ;

    auto session = topicClient.CreateWriteSession(settings);

    std::optional<NYdb::NTopic::TWriteSessionEvent::TEvent> event = session->GetEvent(/*block=*/true);
    NYdb::NTopic::TWriteMessage message("This is yet another message");
    message.MessageMeta({
        {"meta-key", "meta-value"},
        {"another-key", "value"}
    });

    if (auto* readyEvent = std::get_if<NYdb::NTopic::TWriteSessionEvent::TReadyToAcceptEvent>(&*event)) {
        session->Write(std::move(readyEvent->ContinuationToken), std::move(message));
    }
    // [END topic_write_metadata]
    Require(session->Close(TDuration::Seconds(30)), "Metadata writer did not close");
}

void WriteAck(TTopicClient& topicClient, const std::string& path) {
    SeqNumbers ackedSeqNo;
    auto settings = TWriteSessionSettings().Path(path).ProducerId("ack-writer").MessageGroupId("ack-writer");
    // [BEGIN topic_write_ack]
    settings
      // other settings are set here
      .EventHandlers(
        NYdb::NTopic::TWriteSessionSettings::TEventHandlers()
          .AcksHandler(
            [&](NYdb::NTopic::TWriteSessionEvent::TAcksEvent& event) {
              for (const auto& ack : event.Acks) {
                if (ack.State == NYdb::NTopic::TWriteSessionEvent::TWriteAck::EEventState::EES_WRITTEN) {
                  ackedSeqNo.insert(ack.SeqNo);
                  std::cout << "Acknowledged message with seqNo " << ack.SeqNo << std::endl;
                }
              }
            }
          )
      );

    auto session = topicClient.CreateWriteSession(settings);
    // [END topic_write_ack]
    session->Write(NextToken(session), TWriteMessage("acknowledged"));
    Require(session->Close(TDuration::Seconds(30)), "Acknowledged writer did not close");
    Require(ackedSeqNo.size() == 1, "The acknowledgment callback did not receive the message");
}

void WriteNoDedup(TTopicClient& topicClient, const std::string& path) {
    {
        // [BEGIN topic_no_dedup]
        auto settings = NYdb::NTopic::TWriteSessionSettings()
            .Path(path).DeduplicationEnabled(false);

        auto session = topicClient.CreateWriteSession(settings);
        // [END topic_no_dedup]
        session->Write(NextToken(session), TWriteMessage("no_dedup_async"));
        Require(session->Close(TDuration::Seconds(30)), "Writer without deduplication did not close");
    }
    {
        // [BEGIN topic_no_dedup_blocking]
        auto settings = NYdb::NTopic::TWriteSessionSettings()
            .Path(path).DeduplicationEnabled(false);

        auto session = topicClient.CreateSimpleBlockingWriteSession(settings);
        // [END topic_no_dedup_blocking]
        Require(session->Write(TWriteMessage("no_dedup_blocking")), "Write without deduplication failed");
        Require(session->Close(TDuration::Seconds(30)), "Writer without deduplication did not close");
    }
}

void Codecs(TTopicClient& topicClient, const std::string& path) {
    {
        auto settings = TWriteSessionSettings().Path(path).ProducerId("codec-async").MessageGroupId("codec-async");
        // [BEGIN topic_codec]
        settings
          // other settings are set here
          .Codec(ECodec::RAW);

        auto session = topicClient.CreateWriteSession(settings);
        // [END topic_codec]
        session->Write(NextToken(session), TWriteMessage("codec_async"));
        Require(session->Close(TDuration::Seconds(30)), "Codec writer did not close");
    }
    {
        auto settings = TWriteSessionSettings().Path(path).ProducerId("codec-blocking").MessageGroupId("codec-blocking");
        // [BEGIN topic_codec_blocking]
        settings
            // other settings are set here
            .Codec(ECodec::RAW);

        auto session = topicClient.CreateSimpleBlockingWriteSession(settings);
        // [END topic_codec_blocking]
        Require(session->Write(TWriteMessage("codec_blocking")), "Codec write failed");
        Require(session->Close(TDuration::Seconds(30)), "Codec writer did not close");
    }
}

void Producer(TTopicClient& ignored, const std::string& path) {
    (void)ignored;
    TTopicClient topicClient(*ProducerDriver, TTopicClientSettings()
        .DefaultHandlersExecutor(NYdb::CreateThreadPoolExecutor(1)));
    // [BEGIN topic_start_producer]
    NYdb::NTopic::TProducerSettings producerSettings;
    producerSettings.Path(path);
    producerSettings.ProducerIdPrefix("my-producer")
        .PartitionChooserStrategy(NYdb::NTopic::TProducerSettings::EPartitionChooserStrategy::Bound);

    auto producer = topicClient.CreateProducer(producerSettings);
    // [END topic_start_producer]
    {
    // [BEGIN topic_write_producer]
    auto messageData = std::string("order-created");
    // First argument is the partitioning key — the SDK chooses a partition by it.
    NYdb::NTopic::TWriteMessage writeMessage("user-42", messageData);
    producer->Write(std::move(writeMessage));
    producer->Flush().GetValueSync();
    // [END topic_write_producer]
    }
    {
    // [BEGIN topic_write_metadata_producer]
    auto messageData = std::string("message-data");
    NYdb::NTopic::TWriteMessage writeMessage("user-42", messageData);
    writeMessage.MessageMeta({
        {"meta-key", "meta-value"},
        {"another-key", "value"},
    });
    producer->Write(std::move(writeMessage));
    // [END topic_write_metadata_producer]
    }
    Require(producer->Flush().GetValueSync().IsSuccess(), "Producer did not flush");
    Require(producer->Close(TDuration::Seconds(30)).IsSuccess(), "Producer did not close");
}

void ProducerAck(TTopicClient& ignored, const std::string& path) {
    (void)ignored;
    TTopicClient topicClient(*ProducerDriver, TTopicClientSettings()
        .DefaultHandlersExecutor(NYdb::CreateThreadPoolExecutor(1)));
    // [BEGIN topic_producer_ack]
    NYdb::NTopic::TProducerSettings producerSettings;
    producerSettings.Path(path);
    producerSettings.ProducerIdPrefix("my-producer")
        .EventHandlers(
            NYdb::NTopic::TWriteSessionSettings::TEventHandlers()
                .AcksHandler([](NYdb::NTopic::TWriteSessionEvent::TAcksEvent& event) {
                    // handle acknowledgements
                    (void)event;
                })
        );
    auto producer = topicClient.CreateProducer(producerSettings);
    // [END topic_producer_ack]
    Require(producer->Write(TWriteMessage("user-42", "producer_ack")).IsQueued(), "Producer write failed");
    Require(producer->Flush().GetValueSync().IsSuccess(), "Producer did not flush");
    Require(producer->Close(TDuration::Seconds(30)).IsSuccess(), "Producer did not close");
}

void ProducerCodec(TTopicClient& ignored, const std::string& path) {
    (void)ignored;
    TTopicClient topicClient(*ProducerDriver, TTopicClientSettings()
        .DefaultHandlersExecutor(NYdb::CreateThreadPoolExecutor(1)));
    TProducerSettings producerSettings;
    producerSettings.Path(path);
    producerSettings.ProducerIdPrefix("codec-producer");
    // [BEGIN topic_codec_producer]
    producerSettings
        // other settings are set here
        .Codec(NYdb::NTopic::ECodec::RAW);

    auto producer = topicClient.CreateProducer(producerSettings);
    // [END topic_codec_producer]
    Require(producer->Write(TWriteMessage("user-42", "codec_producer")).IsQueued(), "Producer write failed");
    Require(producer->Flush().GetValueSync().IsSuccess(), "Producer did not flush");
    Require(producer->Close(TDuration::Seconds(30)).IsSuccess(), "Producer did not close");
}

void Record(TReadSessionEvent::TDataReceivedEvent& event, std::map<std::string, size_t>& received,
            const std::map<std::string, size_t>& expected) {
    for (const auto& message : event.GetMessages()) {
        const auto& payload = message.GetData();
        Require(expected.count(payload) != 0, "Unexpected payload: " + payload);
        ++received[payload];
        if (payload == "message-data" || payload == "This is yet another message") {
            Require(message.GetMessageMeta() != nullptr, "Message metadata is missing");
            const auto& fields = message.GetMessageMeta()->Fields;
            Require(std::find(fields.begin(), fields.end(),
                std::pair<std::string, std::string>{"meta-key", "meta-value"}) != fields.end(), "Unexpected metadata");
        }
    }
}

void HandleControl(TReadSessionEvent::TEvent& event, std::map<uint64_t, uint64_t>* offsets = nullptr) {
    if (auto* start = std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(&event)) {
        if (offsets) {
            start->Confirm((*offsets)[start->GetPartitionSession()->GetPartitionId()]);
        } else {
            start->Confirm();
        }
    } else if (auto* stop = std::get_if<TReadSessionEvent::TStopPartitionSessionEvent>(&event)) {
        stop->Confirm();
    } else if (auto* closed = std::get_if<TReadSessionEvent::TPartitionSessionClosedEvent>(&event)) {
        if (closed->GetReason() == TReadSessionEvent::TPartitionSessionClosedEvent::EReason::ConnectionLost) {
            std::cout << "The partition connection was lost" << std::endl;
        }
    } else if (auto* end = std::get_if<TReadSessionEvent::TEndPartitionSessionEvent>(&event)) {
        end->Confirm();
    } else if (auto* closed = std::get_if<TSessionClosedEvent>(&event)) {
        ThrowOnError(*closed);
        throw std::runtime_error("Read session closed before receiving the expected messages");
    }
}

void Read(TTopicClient& client, const std::string& path, const std::string& consumer,
          bool ownOffsets = false, bool withoutConsumer = false,
          const std::map<std::string, size_t>& expected = Expected) {
    std::shared_ptr<IReadSession> readSession;
    std::map<uint64_t, uint64_t> offsets;
    std::mutex offsetMutex;
    auto GetOffsetToReadFrom = [&](uint64_t partition) {
        std::lock_guard lock(offsetMutex);
        return offsets[partition];
    };
    if (withoutConsumer) {
        // [BEGIN topic_no_consumer]
        auto settings = NYdb::NTopic::TReadSessionSettings()
            .WithoutConsumer()
            .AppendTopics(NYdb::NTopic::TTopicReadSettings(path)
                .AppendPartitionIds(0).AppendPartitionIds(1).AppendPartitionIds(2));
        auto session = client.CreateReadSession(settings);
        // [END topic_no_consumer]
        readSession = std::move(session);
        ownOffsets = true;
    } else if (ownOffsets) {
        auto settings = TReadSessionSettings().ConsumerName(consumer).AppendTopics(path);
        // [BEGIN topic_client_offset]
        settings.EventHandlers_.StartPartitionSessionHandler(
            [&](NYdb::NTopic::TReadSessionEvent::TStartPartitionSessionEvent& event) {
                auto readFromOffset = GetOffsetToReadFrom(event.GetPartitionSession()->GetPartitionId());
                event.Confirm(readFromOffset);
            }
        );
        // [END topic_client_offset]
        readSession = client.CreateReadSession(settings);
    } else {
        // [BEGIN topic_start_reader]
        auto settings = NYdb::NTopic::TReadSessionSettings()
            .ConsumerName(consumer)
            .AppendTopics(path);
        auto session = client.CreateReadSession(settings);
        // [END topic_start_reader]
        readSession = std::move(session);
    }
    std::map<std::string, size_t> received;
    while (Total(received) < Total(expected)) {
        auto event = readSession->GetEvent(true);
        Require(event.has_value(), "Read session closed unexpectedly");
        if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&*event)) {
            Record(*data, received, expected);
            if (ownOffsets) {
                std::lock_guard lock(offsetMutex);
                for (const auto& message : data->GetMessages()) {
                    offsets[message.GetPartitionSession()->GetPartitionId()] = message.GetOffset() + 1;
                }
            }
        } else {
            HandleControl(*event, ownOffsets ? &offsets : nullptr);
        }
    }
    Require(received == expected, "Unexpected message counts");
    Require(readSession->Close(TDuration::Seconds(30)), "Read session did not close");
}

void ReadCallbacks(TTopicClient& topicClient, const std::string& path, const std::string& consumer, bool commit) {
    std::mutex mutex;
    std::map<std::string, size_t> received;
    std::promise<void> done;
    auto future = done.get_future();
    bool completed = false;
    TReadSessionSettings settings;
    if (commit) {
        settings.ConsumerName(consumer).AppendTopics(path);
        // [BEGIN topic_read_batch_commit]
        settings.EventHandlers_.SimpleDataHandlers(
            [](NYdb::NTopic::TReadSessionEvent::TDataReceivedEvent& event) {
                std::cout << "Get data event " << NYdb::NTopic::DebugString(event);
            }, /* commitDataAfterProcessing = */true
        );
        // [END topic_read_batch_commit]
    } else {
        settings.ConsumerName(consumer).AppendTopics(path);
        // [BEGIN topic_read_batch]
        settings.EventHandlers_.SimpleDataHandlers(
            [](NYdb::NTopic::TReadSessionEvent::TDataReceivedEvent& event) {
                std::cout << "Get data event " << NYdb::NTopic::DebugString(event);
            }
        );
        // [END topic_read_batch]
    }
    auto documentedHandler = settings.EventHandlers_.DataReceivedHandler_;
    settings.EventHandlers_.DataReceivedHandler([&](TReadSessionEvent::TDataReceivedEvent& event) {
        std::lock_guard lock(mutex);
        try {
            documentedHandler(event);
            Record(event, received, Expected);
            if (Total(received) == Total(Expected) && !completed) {
                completed = true;
                done.set_value();
            }
        } catch (...) {
            if (!completed) { completed = true; done.set_exception(std::current_exception()); }
        }
    });
    std::shared_ptr<IReadSession> session;
    if (commit) {
        // [BEGIN topic_read_batch_commit_session]
        session = topicClient.CreateReadSession(settings);
        // [END topic_read_batch_commit_session]
    } else {
        // [BEGIN topic_read_batch_session]
        session = topicClient.CreateReadSession(settings);
        // [END topic_read_batch_session]
    }
    auto closeTask = std::async(std::launch::async, [&] {
        if (future.wait_for(std::chrono::seconds(30)) != std::future_status::ready) {
            session->Close(TDuration::Zero());
            throw std::runtime_error("Reader callback timed out");
        }
        try { future.get(); } catch (...) { session->Close(TDuration::Zero()); throw; }
        Require(session->Close(TDuration::Seconds(30)), "Callback session did not close");
    });
    if (commit) {
        // [BEGIN topic_read_batch_commit_wait]
        // Wait SessionClosed event.
        session->GetEvent(/* block = */true);
        // [END topic_read_batch_commit_wait]
    } else {
        // [BEGIN topic_read_batch_wait]
        // Wait SessionClosed event.
        session->GetEvent(/* block = */true);
        // [END topic_read_batch_wait]
    }
    closeTask.get();
    Require(received == Expected, "Unexpected callback message counts");
}

void ReadSelectors(TTopicClient& client, const std::string& path) {
    const std::string another = path + "_another";
    ThrowOnError(client.CreateTopic(another, TCreateTopicSettings().BeginAddConsumer("selectors")
        .EndAddConsumer()).GetValueSync());
    auto someTimestamp = TInstant::Now() - TDuration::Hours(24);
    // [BEGIN topic_reader_selectors]
    auto settings = NYdb::NTopic::TReadSessionSettings()
        .ConsumerName("selectors")
        .AppendTopics(path)
        .AppendTopics(
            NYdb::NTopic::TTopicReadSettings(another)
                .ReadFromTimestamp(someTimestamp)
        );

    auto session = client.CreateReadSession(settings);
    // [END topic_reader_selectors]
    std::map<std::string, size_t> received;
    while (received.empty()) {
        auto event = session->GetEvent(true);
        Require(event.has_value(), "Selector read session closed unexpectedly");
        if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&*event)) {
            Record(*data, received, Expected);
        } else {
            HandleControl(*event);
        }
    }
    Require(session->Close(TDuration::Seconds(30)), "Selector read session did not close");
    ThrowOnError(client.DropTopic(another).GetValueSync());
}

void AutoFull(TTopicClient& topicClient, const std::string& path) {
    auto settings = TReadSessionSettings().ConsumerName("autoscale").AppendTopics(path);
    // [BEGIN topic_autoscale_reader_full]
    settings.AutoPartitioningSupport(true); // full support is enabled
    auto readSession = topicClient.CreateReadSession(settings);
    // [END topic_autoscale_reader_full]
    auto event = readSession->GetEvent(true);
    if (event) HandleControl(*event);
    Require(readSession->Close(TDuration::Seconds(30)), "Auto-partitioning reader did not close");
}

void AutoCompat(TTopicClient& topicClient, const std::string& path) {
    auto settings = TReadSessionSettings().ConsumerName("autoscale").AppendTopics(path);
    // [BEGIN topic_autoscale_reader_compat]
    settings.AutoPartitioningSupport(false); // compatibility mode is enabled
    auto readSession = topicClient.CreateReadSession(settings);
    // [END topic_autoscale_reader_compat]
    auto event = readSession->GetEvent(true);
    if (event) HandleControl(*event);
    Require(readSession->Close(TDuration::Seconds(30)), "Auto-partitioning reader did not close");
}

void ControlEvents(TTopicClient& topicClient, const std::string& path) {
    {
        auto readSession = topicClient.CreateReadSession(TReadSessionSettings().ConsumerName("controls").AppendTopics(path));
        // [BEGIN topic_soft_stop]
        auto event = readSession->GetEvent(/*block=*/true);
        if (auto* stopPartitionSessionEvent = std::get_if<NYdb::NTopic::TReadSessionEvent::TStopPartitionSessionEvent>(&*event)) {
            stopPartitionSessionEvent->Confirm();
        } else {
          // other event types
        }
        // [END topic_soft_stop]
        if (event && std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(&*event)) HandleControl(*event);
        Require(readSession->Close(TDuration::Seconds(30)), "Control reader did not close");
    }
    {
        auto readSession = topicClient.CreateReadSession(TReadSessionSettings().ConsumerName("controls").AppendTopics(path));
        // [BEGIN topic_hard_stop]
        auto event = readSession->GetEvent(/*block=*/true);
        if (auto* partitionSessionClosedEvent = std::get_if<NYdb::NTopic::TReadSessionEvent::TPartitionSessionClosedEvent>(&*event)) {
            if (partitionSessionClosedEvent->GetReason() == NYdb::NTopic::TReadSessionEvent::TPartitionSessionClosedEvent::EReason::ConnectionLost) {
                std::cout << "Connection with partition was lost" << std::endl;
            }
        } else {
          // other event types
        }
        // [END topic_hard_stop]
        if (event && std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(&*event)) HandleControl(*event);
        Require(readSession->Close(TDuration::Seconds(30)), "Control reader did not close");
    }
    {
        TReadSessionSettings settings;
        settings.ConsumerName("auto_end").AppendTopics(path);
        // [BEGIN topic_autoscale_end]
        settings.AutoPartitioningSupport(true);

        auto readSession = topicClient.CreateReadSession(settings);

        auto event = readSession->GetEvent(/*block=*/true);
        if (auto* endPartitionSessionEvent = std::get_if<NYdb::NTopic::TReadSessionEvent::TEndPartitionSessionEvent>(&*event)) {
            endPartitionSessionEvent->Confirm();
        } else {
          // other event types
        }
        // [END topic_autoscale_end]
        if (event && std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(&*event)) HandleControl(*event);
        Require(readSession->Close(TDuration::Seconds(30)), "End-partition reader did not close");
    }
}

void Autoscaling(TTopicClient& client, const std::string& path) {
    AutoFull(client, path);
    AutoCompat(client, path);
    ControlEvents(client, path);
}

void CommitOutside(TTopicClient& client, const std::string& path) {
    auto readSession = client.CreateReadSession(TReadSessionSettings().ConsumerName("outside").AppendTopics(path));
    std::optional<uint64_t> offset;
    uint64_t partitionId = 0;
    while (!offset) {
        auto event = readSession->GetEvent(true);
        Require(event.has_value(), "Read session closed unexpectedly");
        if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&*event)) {
            if (!data->GetMessages().empty()) {
                offset = data->GetMessages().back().GetOffset() + 1;
                partitionId = data->GetPartitionSession()->GetPartitionId();
            }
        } else {
            HandleControl(*event);
        }
    }
    // [BEGIN topic_commit_outside_session]
    auto description = client.DescribeConsumer(path, "outside", TDescribeConsumerSettings().IncludeStats(true))
        .GetValueSync();
    ThrowOnError(description);
    std::string serverReadSessionId;
    for (const auto& partition : description.GetConsumerDescription().GetPartitions()) {
        const auto& stats = partition.GetPartitionConsumerStats();
        if (partition.GetPartitionId() == partitionId && stats) {
            serverReadSessionId = stats->GetReadSessionId();
            break;
        }
    }
    Require(!serverReadSessionId.empty(), "Active server read session ID is missing");
    ThrowOnError(client.CommitOffset(path, partitionId, "outside", *offset,
        TCommitOffsetSettings().ReadSessionId(serverReadSessionId)).GetValueSync());
    // [END topic_commit_outside_session]
    Require(readSession->Close(TDuration::Seconds(30)), "Read session did not close");
    // [BEGIN topic_commit_outside]
    NYdb::NStatusHelpers::ThrowOnError(client.CommitOffset(
        path, partitionId, "outside", *offset).GetValueSync());
    // [END topic_commit_outside]
}

class ObservedReadSession final : public IReadSession {
public:
    ObservedReadSession(std::shared_ptr<IReadSession> inner, std::map<std::string, size_t>& received,
                        const std::map<std::string, size_t>& expected)
        : Inner(std::move(inner)), Received(received), ExpectedMessages(expected) {}
    NThreading::TFuture<void> WaitEvent() override { return Inner->WaitEvent(); }
    std::vector<TReadSessionEvent::TEvent> GetEvents(bool block = false, std::optional<size_t> count = std::nullopt,
            size_t bytes = std::numeric_limits<size_t>::max()) override {
        auto events = Inner->GetEvents(block, count, bytes);
        for (auto& event : events) Observe(event);
        return events;
    }
    std::vector<TReadSessionEvent::TEvent> GetEvents(const TReadSessionGetEventSettings& settings) override {
        auto events = Inner->GetEvents(settings);
        for (auto& event : events) Observe(event);
        return events;
    }
    std::optional<TReadSessionEvent::TEvent> GetEvent(bool block = false,
            size_t bytes = std::numeric_limits<size_t>::max()) override { return Inner->GetEvent(block, bytes); }
    std::optional<TReadSessionEvent::TEvent> GetEvent(const TReadSessionGetEventSettings& settings) override {
        return Inner->GetEvent(settings);
    }
    bool Close(TDuration timeout = TDuration::Max()) override { return Inner->Close(timeout); }
    TReaderCounters::TPtr GetCounters() const override { return Inner->GetCounters(); }
    std::string GetSessionId() const override { return Inner->GetSessionId(); }
private:
    void Observe(TReadSessionEvent::TEvent& event) {
        if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&event)) Record(*data, Received, ExpectedMessages);
    }
    std::shared_ptr<IReadSession> Inner;
    std::map<std::string, size_t>& Received;
    const std::map<std::string, size_t>& ExpectedMessages;
};

void PrepareRead(const std::shared_ptr<IReadSession>& readSession) {
    auto event = readSession->GetEvent(true);
    Require(event.has_value(), "Read session closed during initialization");
    HandleControl(*event);
    Require(readSession->WaitEvent().Wait(TDuration::Seconds(30)), "Transactional message did not arrive");
}

void Transactions(TTopicClient& client, NYdb::NQuery::TQueryClient& queryClient, const std::string& path) {
    ThrowOnError(client.CreateTopic(path, TCreateTopicSettings().BeginAddConsumer("transaction")
        .EndAddConsumer().BeginAddConsumer("deferred_stop").EndAddConsumer()).GetValueSync());
    {
        auto topicSession = client.CreateWriteSession(TWriteSessionSettings().Path(path)
            .ProducerId("tx-async").MessageGroupId("tx-async"));
        // [BEGIN topic_write_tx]
        NYdb::NStatusHelpers::ThrowOnError(queryClient.RetryQuerySync([&](NYdb::NQuery::TSession session) -> NYdb::TStatus {
            auto beginTxResult = session.BeginTransaction(NYdb::NQuery::TTxSettings::SerializableRW()).GetValueSync();
            if (!beginTxResult.IsSuccess()) { return beginTxResult; }
            auto tx = beginTxResult.GetTransaction();
            NYdb::NTopic::TWriteMessage writeMessage("message");
            topicSession->Write(NextToken(topicSession), std::move(writeMessage), &tx);
            topicSession->Flush().GetValueSync();
            return tx.Commit().GetValueSync();
        }));
        // [END topic_write_tx]
        Require(topicSession->Close(TDuration::Seconds(30)), "Transactional writer did not close");
    }
    {
        auto topicSession = client.CreateSimpleBlockingWriteSession(TWriteSessionSettings().Path(path)
            .ProducerId("tx-blocking").MessageGroupId("tx-blocking"));
        // [BEGIN topic_write_tx_blocking]
        NYdb::NStatusHelpers::ThrowOnError(queryClient.RetryQuerySync([&](NYdb::NQuery::TSession session) -> NYdb::TStatus {
            auto beginTxResult = session.BeginTransaction(NYdb::NQuery::TTxSettings::SerializableRW()).GetValueSync();
            if (!beginTxResult.IsSuccess()) { return beginTxResult; }
            auto tx = beginTxResult.GetTransaction();
            NYdb::NTopic::TWriteMessage writeMessage("message");
            topicSession->Write(std::move(writeMessage), &tx);
            return tx.Commit().GetValueSync();
        }));
        // [END topic_write_tx_blocking]
        Require(topicSession->Close(TDuration::Seconds(30)), "Transactional writer did not close");
    }
    const std::map<std::string, size_t> expected = {{"message", 2}};
    {
        std::map<std::string, size_t> received;
        auto rawSession = client.CreateReadSession(TReadSessionSettings().ConsumerName("transaction").AppendTopics(path));
        auto readSession = std::make_shared<ObservedReadSession>(rawSession, received, expected);
        PrepareRead(readSession);
        while (Total(received) < Total(expected)) {
            // [BEGIN topic_read_tx]
            readSession->WaitEvent().Wait(TDuration::Seconds(1));

            NYdb::NStatusHelpers::ThrowOnError(queryClient.RetryQuerySync([&readSession](NYdb::NQuery::TSession session) -> NYdb::TStatus {
                auto beginTxResult = session.BeginTransaction(NYdb::NQuery::TTxSettings::SerializableRW()).GetValueSync();
                if (!beginTxResult.IsSuccess()) {
                    return beginTxResult;
                }
                auto tx = beginTxResult.GetTransaction();

                auto topicSettings = NYdb::NTopic::TReadSessionGetEventSettings()
                    .Block(false)
                    .Tx(tx);

                auto events = readSession->GetEvents(topicSettings);

                for (auto& event : events) {
                    // process the event and write results to the table
                }

                return tx.Commit().GetValueSync();
            }));
            // [END topic_read_tx]
        }
        Require(received == expected, "Unexpected transactional messages");
        Require(readSession->Close(TDuration::Seconds(30)), "Transactional reader did not close");
    }
    {
        std::map<std::string, size_t> received;
        auto rawSession = client.CreateReadSession(TReadSessionSettings().ConsumerName("deferred_stop").AppendTopics(path));
        auto readSession = std::make_shared<ObservedReadSession>(rawSession, received, expected);
        PrepareRead(readSession);
        while (Total(received) < Total(expected)) {
            ThrowOnError(queryClient.RetryQuerySync([&](NYdb::NQuery::TSession session) -> NYdb::TStatus {
                auto beginTxResult = session.BeginTransaction(NYdb::NQuery::TTxSettings::SerializableRW()).GetValueSync();
                ThrowOnError(beginTxResult);
                auto tx = beginTxResult.GetTransaction();
                auto topicSettings = TReadSessionGetEventSettings().Block(true).Tx(tx);
                NYdb::NQuery::TCommitTxSettings commitSettings;
                // [BEGIN topic_read_tx_deferred_stop]
                std::optional<NYdb::NTopic::TReadSessionEvent::TStopPartitionSessionEvent> stopPartitionSessionEvent;

                auto events = readSession->GetEvents(topicSettings);

                for (auto& event : events) {
                    if (auto* e = std::get_if<NYdb::NTopic::TReadSessionEvent::TStopPartitionSessionEvent>(&event)) {
                        stopPartitionSessionEvent = std::move(*e);
                    } else {
                        // process the event and write results to the table
                    }
                }

                auto commitResult = tx.Commit(commitSettings).GetValueSync();
                if (!commitResult.IsSuccess()) {
                    return commitResult;
                }

                if (stopPartitionSessionEvent) {
                    stopPartitionSessionEvent->Confirm();
                }
                // [END topic_read_tx_deferred_stop]
                return commitResult;
            }));
        }
        Require(received == expected, "Unexpected deferred-stop messages");
        Require(readSession->Close(TDuration::Seconds(30)), "Deferred-stop reader did not close");
    }
    ThrowOnError(client.DropTopic(path).GetValueSync());
}

void Run() {
    struct {
        std::string Endpoint;
        std::string Database;
    } opts{Environment("YDB_ENDPOINT", "localhost:2136"), Environment("YDB_DATABASE", "/local")};
    // [BEGIN topic_init]
    // Create driver instance.
    auto driverConfig = NYdb::TDriverConfig()
        .SetEndpoint(opts.Endpoint)
        .SetDatabase(opts.Database)
        .SetAuthToken(Environment("YDB_TOKEN", ""));

    NYdb::TDriver driver(driverConfig);
    // [END topic_init]
    // [BEGIN topic_client]
    NYdb::NTopic::TTopicClient topicClient(driver);
    // [END topic_client]
    ProducerDriver = &driver;
    NYdb::NQuery::TQueryClient queryClient(driver);
    const std::string path = "ydb_tech_" + std::to_string(TInstant::Now().MicroSeconds());
    // [BEGIN topic_create]
    auto settings = NYdb::NTopic::TCreateTopicSettings()
        .PartitioningSettings(3, 3)
        .AppendSupportedCodecs(NYdb::NTopic::ECodec::ZSTD);

    auto status = topicClient
        .CreateTopic(path, settings)  // returns TFuture<TStatus>
        .GetValueSync();
    // [END topic_create]
    ThrowOnError(status);
    const std::string producerPath = path + "_producer";
    ThrowOnError(topicClient.CreateTopic(producerPath, TCreateTopicSettings()
        .PartitioningSettings(TPartitioningSettings(3, 4,
            TAutoPartitioningSettings(EAutoPartitioningStrategy::ScaleUp, TDuration::Seconds(300), 20, 80)))
        .BeginAddConsumer("producer").EndAddConsumer()).GetValueSync());
    try {
        TAlterTopicSettings preparation;
        preparation.SetSupportedCodecs({ECodec::RAW, ECodec::GZIP, ECodec::ZSTD});
        for (const auto& name : {"poll", "batch", "commit_batch", "offset", "selectors", "autoscale", "auto_end", "outside", "controls"}) {
            preparation.BeginAddConsumer(name).EndAddConsumer();
        }
        ThrowOnError(topicClient.AlterTopic(path, preparation).GetValueSync());
        {
            // [BEGIN topic_alter]
            auto alterSettings = NYdb::NTopic::TAlterTopicSettings()
                .BeginAddConsumer("my-consumer")
                    .SetImportant(true)
                .EndAddConsumer()
                .SetRetentionPeriod(TDuration::Days(2));

            auto status = topicClient
                .AlterTopic(path, alterSettings)  // returns TFuture<TStatus>
                .GetValueSync();
            // [END topic_alter]
            ThrowOnError(status);
        }
        {
            // [BEGIN topic_describe]
            auto result = topicClient.DescribeTopic(path).GetValueSync();
            if (result.IsSuccess()) {
                const auto& description = result.GetTopicDescription();
                std::cout << "Topic description: " << NYdb::TProtoAccessor::GetProto(description).DebugString() << std::endl;
            }
            // [END topic_describe]
            ThrowOnError(result);
        }
        WriteAsync(topicClient, path);
        WriteBlocking(topicClient, path);
        WriteMetadata(topicClient, path);
        WriteAck(topicClient, path);
        WriteNoDedup(topicClient, path);
        Codecs(topicClient, path);
        Producer(topicClient, producerPath);
        ProducerAck(topicClient, producerPath);
        ProducerCodec(topicClient, producerPath);
        Read(topicClient, path, "my-consumer");
        Read(topicClient, producerPath, "producer", false, false,
            {{"order-created", 1}, {"message-data", 1}, {"producer_ack", 1}, {"codec_producer", 1}});
        Read(topicClient, path, "offset", true);
        Read(topicClient, path, "", false, true);
        ReadCallbacks(topicClient, path, "batch", false);
        ReadCallbacks(topicClient, path, "commit_batch", true);
        ReadSelectors(topicClient, path);
        Autoscaling(topicClient, path);
        CommitOutside(topicClient, path);
        Transactions(topicClient, queryClient, path + "_tx");
    } catch (...) {
        topicClient.DropTopic(producerPath).GetValueSync();
        topicClient.DropTopic(path).GetValueSync();
        throw;
    }
    ThrowOnError(topicClient.DropTopic(producerPath).GetValueSync());
    {
        // [BEGIN topic_drop]
        auto status = topicClient.DropTopic(path).GetValueSync();
        // [END topic_drop]
        ThrowOnError(status);
    }
    driver.Stop(true);
}

} // namespace

int main() {
    try {
        Run();
        std::cout << "All topic scenarios completed" << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
