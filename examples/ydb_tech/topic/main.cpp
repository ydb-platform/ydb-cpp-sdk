#include <ydb-cpp-sdk/client/driver/driver.h>
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
    {"blocking", 1}, {"blocking_metadata", 1}, {"async", 1}, {"async_metadata", 1},
    {"producer", 1}, {"producer_metadata", 1}, {"no_dedup_async", 1}, {"no_dedup_blocking", 1},
    {"codec_async", 1}, {"codec_blocking", 1}, {"codec_producer", 1},
};

// [BEGIN topic_next_token]
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
// [END topic_next_token]

void Write(TTopicClient& client, const std::string& path) {
    std::atomic<size_t> acknowledged{0};
    TWriteSessionSettings settings;
    // [BEGIN topic_write_ack]
    settings.EventHandlers_.AcksHandler([&](TWriteSessionEvent::TAcksEvent& event) {
        for (const auto& ack : event.Acks) {
            if (ack.State == TWriteSessionEvent::TWriteAck::EEventState::EES_WRITTEN) {
                acknowledged.fetch_add(1);
            }
        }
    });
    // [END topic_write_ack]
    // [BEGIN topic_start_writer]
    settings.Path(path).ProducerId("ydb-tech-async").MessageGroupId("ydb-tech-async");
    auto session = client.CreateWriteSession(settings);
    // [END topic_start_writer]
    // [BEGIN topic_write]
    auto token = NextToken(session);
    session->Write(std::move(token), TWriteMessage("async"));
    // [END topic_write]
    // [BEGIN topic_write_metadata]
    TWriteMessage message("async_metadata");
    message.MessageMeta({{"meta-key", "meta-value"}});
    session->Write(NextToken(session), std::move(message));
    // [END topic_write_metadata]
    Require(session->Flush().GetValueSync(), "The asynchronous writer did not flush");
    Require(session->Close(TDuration::Seconds(30)), "The asynchronous writer did not close");
    Require(acknowledged.load() == 2, "Unexpected acknowledgment count");

    // [BEGIN topic_start_writer_blocking]
    auto blockingSettings = TWriteSessionSettings()
        .Path(path).ProducerId("ydb-tech-blocking").MessageGroupId("ydb-tech-blocking");
    auto blocking = client.CreateSimpleBlockingWriteSession(blockingSettings);
    // [END topic_start_writer_blocking]
    // [BEGIN topic_write_blocking]
    Require(blocking->Write(TWriteMessage("blocking"), nullptr, TDuration::Seconds(30)), "Write failed");
    // [END topic_write_blocking]
    // [BEGIN topic_write_metadata_blocking]
    TWriteMessage blockingMessage("blocking_metadata");
    blockingMessage.MessageMeta({{"meta-key", "meta-value"}});
    Require(blocking->Write(std::move(blockingMessage), nullptr, TDuration::Seconds(30)), "Write failed");
    // [END topic_write_metadata_blocking]
    Require(blocking->Close(TDuration::Seconds(30)), "The blocking writer did not close");

    TProducerSettings producerSettings;
    // A producer stops its handler executor on destruction; keep it separate from other sessions.
    producerSettings.EventHandlers_.HandlersExecutor(NYdb::CreateThreadPoolExecutor(1));
    // [BEGIN topic_producer_ack]
    producerSettings.EventHandlers_.AcksHandler([&](TWriteSessionEvent::TAcksEvent& event) {
        for (const auto& ack : event.Acks) {
            if (ack.State == TWriteSessionEvent::TWriteAck::EEventState::EES_WRITTEN) {
                acknowledged.fetch_add(1);
            }
        }
    });
    // [END topic_producer_ack]
    // [BEGIN topic_start_producer]
    producerSettings.Path(path);
    producerSettings.ProducerIdPrefix("ydb-tech-producer")
        .PartitionChooserStrategy(TProducerSettings::EPartitionChooserStrategy::KafkaHash);
    auto producer = client.CreateProducer(producerSettings);
    // [END topic_start_producer]
    // [BEGIN topic_write_producer]
    Require(producer->Write(TWriteMessage("user-42", "producer")).IsQueued(), "Producer write failed");
    Require(producer->Flush().GetValueSync().IsSuccess(), "Producer flush failed");
    // [END topic_write_producer]
    // [BEGIN topic_write_metadata_producer]
    TWriteMessage producerMessage("user-42", "producer_metadata");
    producerMessage.MessageMeta({{"meta-key", "meta-value"}});
    Require(producer->Write(std::move(producerMessage)).IsQueued(), "Producer write failed");
    // [END topic_write_metadata_producer]
    Require(producer->Flush().GetValueSync().IsSuccess(), "Producer flush failed");
    Require(producer->Close(TDuration::Seconds(30)).IsSuccess(), "Producer did not close");

    // [BEGIN topic_no_dedup]
    auto noDedup = client.CreateWriteSession(TWriteSessionSettings().Path(path).DeduplicationEnabled(false));
    noDedup->Write(NextToken(noDedup), TWriteMessage("no_dedup_async"));
    // [END topic_no_dedup]
    Require(noDedup->Close(TDuration::Seconds(30)), "The writer without deduplication did not close");
    // [BEGIN topic_no_dedup_blocking]
    auto noDedupBlocking = client.CreateSimpleBlockingWriteSession(
        TWriteSessionSettings().Path(path).DeduplicationEnabled(false));
    Require(noDedupBlocking->Write(TWriteMessage("no_dedup_blocking"), nullptr, TDuration::Seconds(30)), "Write failed");
    // [END topic_no_dedup_blocking]
    Require(noDedupBlocking->Close(TDuration::Seconds(30)), "The writer without deduplication did not close");
}

void Codecs(TTopicClient& client, const std::string& path) {
    // [BEGIN topic_codec]
    auto session = client.CreateWriteSession(TWriteSessionSettings()
        .Path(path).ProducerId("codec-async").MessageGroupId("codec-async").Codec(ECodec::RAW));
    // [END topic_codec]
    session->Write(NextToken(session), TWriteMessage("codec_async"));
    Require(session->Close(TDuration::Seconds(30)), "Codec writer did not close");
    // [BEGIN topic_codec_blocking]
    auto blocking = client.CreateSimpleBlockingWriteSession(TWriteSessionSettings()
        .Path(path).ProducerId("codec-blocking").MessageGroupId("codec-blocking").Codec(ECodec::GZIP));
    // [END topic_codec_blocking]
    Require(blocking->Write(TWriteMessage("codec_blocking"), nullptr, TDuration::Seconds(30)), "Codec write failed");
    Require(blocking->Close(TDuration::Seconds(30)), "Codec writer did not close");
    // [BEGIN topic_codec_producer]
    TProducerSettings settings;
    settings.EventHandlers_.HandlersExecutor(NYdb::CreateThreadPoolExecutor(1));
    settings.Path(path).Codec(ECodec::GZIP);
    settings.ProducerIdPrefix("codec-producer")
        .PartitionChooserStrategy(TProducerSettings::EPartitionChooserStrategy::KafkaHash);
    auto producer = client.CreateProducer(settings);
    // [END topic_codec_producer]
    Require(producer->Write(TWriteMessage("user-42", "codec_producer")).IsQueued(), "Codec write failed");
    Require(producer->Flush().GetValueSync().IsSuccess(), "Codec producer flush failed");
    Require(producer->Close(TDuration::Seconds(30)).IsSuccess(), "Codec producer did not close");
}

void Record(TReadSessionEvent::TDataReceivedEvent& event, std::map<std::string, size_t>& received,
            const std::map<std::string, size_t>& expected) {
    for (const auto& message : event.GetMessages()) {
        const auto& payload = message.GetData();
        Require(expected.count(payload) != 0, "Unexpected payload: " + payload);
        ++received[payload];
        if (payload.find("metadata") != std::string::npos) {
            Require(message.GetMessageMeta() != nullptr, "Message metadata is missing");
            const auto& fields = message.GetMessageMeta()->Fields;
            Require(std::find(fields.begin(), fields.end(),
                std::pair<std::string, std::string>{"meta-key", "meta-value"}) != fields.end(), "Unexpected metadata");
        }
    }
}

void HandleControl(TReadSessionEvent::TEvent& event, std::map<uint64_t, uint64_t>* offsets = nullptr) {
    if (auto* start = std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(&event)) {
        // [BEGIN topic_client_offset]
        if (offsets) {
            start->Confirm((*offsets)[start->GetPartitionSession()->GetPartitionId()]);
        } else {
            start->Confirm();
        }
        // [END topic_client_offset]
    } else if (auto* stop = std::get_if<TReadSessionEvent::TStopPartitionSessionEvent>(&event)) {
        // [BEGIN topic_soft_stop]
        stop->Confirm();
        // [END topic_soft_stop]
    } else if (auto* closed = std::get_if<TReadSessionEvent::TPartitionSessionClosedEvent>(&event)) {
        // [BEGIN topic_hard_stop]
        if (closed->GetReason() == TReadSessionEvent::TPartitionSessionClosedEvent::EReason::ConnectionLost) {
            std::cout << "The partition connection was lost" << std::endl;
        }
        // [END topic_hard_stop]
    } else if (auto* end = std::get_if<TReadSessionEvent::TEndPartitionSessionEvent>(&event)) {
        // [BEGIN topic_autoscale_end]
        end->Confirm();
        // [END topic_autoscale_end]
    } else if (auto* closed = std::get_if<TSessionClosedEvent>(&event)) {
        ThrowOnError(*closed);
        throw std::runtime_error("Read session closed before receiving the expected messages");
    }
}

void Read(TTopicClient& client, const std::string& path, const std::string& consumer,
          bool ownOffsets = false, bool withoutConsumer = false) {
    std::shared_ptr<IReadSession> readSession;
    if (withoutConsumer) {
        // [BEGIN topic_no_consumer]
        auto settings = TReadSessionSettings().WithoutConsumer().AppendTopics(
            TTopicReadSettings(path).AppendPartitionIds(0).AppendPartitionIds(1).AppendPartitionIds(2));
        auto session = client.CreateReadSession(settings);
        // [END topic_no_consumer]
        readSession = std::move(session);
        ownOffsets = true;
    } else {
        // [BEGIN topic_start_reader]
        auto settings = TReadSessionSettings().ConsumerName(consumer).AppendTopics(path);
        auto session = client.CreateReadSession(settings);
        // [END topic_start_reader]
        readSession = std::move(session);
    }
    std::map<uint64_t, uint64_t> offsets;
    std::map<std::string, size_t> received;
    while (received.size() < Expected.size()) {
        auto event = readSession->GetEvent(true);
        Require(event.has_value(), "Read session closed unexpectedly");
        if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&*event)) {
            Record(*data, received, Expected);
            if (ownOffsets) {
                for (const auto& message : data->GetMessages()) {
                    offsets[message.GetPartitionSession()->GetPartitionId()] = message.GetOffset() + 1;
                }
            }
        } else {
            HandleControl(*event, ownOffsets ? &offsets : nullptr);
        }
    }
    Require(received == Expected, "Unexpected message counts");
    Require(readSession->Close(TDuration::Seconds(30)), "Read session did not close");
}

void ReadCallbacks(TTopicClient& client, const std::string& path, const std::string& consumer, bool commit) {
    std::mutex mutex;
    std::map<std::string, size_t> received;
    std::promise<void> done;
    auto future = done.get_future();
    bool completed = false;
    auto process = [&](TReadSessionEvent::TDataReceivedEvent& event) {
        std::lock_guard<std::mutex> lock(mutex);
        try {
            Record(event, received, Expected);
            if (received.size() == Expected.size() && !completed) {
                completed = true;
                done.set_value();
            }
        } catch (...) {
            if (!completed) {
                completed = true;
                done.set_exception(std::current_exception());
            }
        }
    };
    auto settings = TReadSessionSettings().ConsumerName(consumer).AppendTopics(path);
    if (commit) {
        // [BEGIN topic_read_batch_commit]
        settings.EventHandlers_.SimpleDataHandlers(process, true);
        // [END topic_read_batch_commit]
    } else {
        // [BEGIN topic_read_batch]
        settings.EventHandlers_.SimpleDataHandlers(process, false);
        // [END topic_read_batch]
    }
    auto session = client.CreateReadSession(settings);
    Require(future.wait_for(std::chrono::seconds(30)) == std::future_status::ready, "Reader callback timed out");
    future.get();
    Require(session->Close(TDuration::Seconds(30)), "Callback read session did not close");
    Require(received == Expected, "Unexpected callback message counts");
}

void ReadSelectors(TTopicClient& client, const std::string& path) {
    const std::string another = path + "_another";
    ThrowOnError(client.CreateTopic(another, TCreateTopicSettings().BeginAddConsumer("selectors")
        .EndAddConsumer()).GetValueSync());
    // [BEGIN topic_reader_selectors]
    auto settings = TReadSessionSettings().ConsumerName("selectors").AppendTopics(path)
        .AppendTopics(TTopicReadSettings(another).ReadFromTimestamp(TInstant::Zero()));
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

void Autoscaling(TTopicClient& client, const std::string& path) {
    // [BEGIN topic_autoscale_reader]
    for (bool fullSupport : {true, false}) {
        auto settings = TReadSessionSettings().ConsumerName("autoscale").AppendTopics(path)
            .AutoPartitioningSupport(fullSupport);
        auto session = client.CreateReadSession(settings);
        std::map<std::string, size_t> received;
        while (received.empty()) {
            auto event = session->GetEvent(true);
            Require(event.has_value(), "Autoscaling read session closed unexpectedly");
            if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&*event)) {
                Record(*data, received, Expected);
            } else {
                HandleControl(*event);
            }
        }
        Require(session->Close(TDuration::Seconds(30)), "Autoscaling read session did not close");
    }
    // [END topic_autoscale_reader]
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
    ThrowOnError(client.CommitOffset(path, partitionId, "outside", *offset).GetValueSync());
    // [END topic_commit_outside]
}

// [BEGIN topic_read_tx_deferred_stop]
NYdb::TStatus CommitAndConfirm(NYdb::NQuery::TTransaction& tx,
        std::vector<TReadSessionEvent::TStopPartitionSessionEvent>& stoppedPartitions) {
    auto result = tx.Commit().GetValueSync();
    if (!result.IsSuccess()) {
        return result;
    }
    for (auto& event : stoppedPartitions) {
        event.Confirm();
    }
    return result;
}
// [END topic_read_tx_deferred_stop]

void Transactions(TTopicClient& client, NYdb::NQuery::TQueryClient& queryClient, const std::string& path) {
    ThrowOnError(client.CreateTopic(path, TCreateTopicSettings().BeginAddConsumer("transaction")
        .EndAddConsumer().BeginAddConsumer("deferred_stop").EndAddConsumer()).GetValueSync());
    auto session = client.CreateWriteSession(TWriteSessionSettings().Path(path)
        .ProducerId("tx-async").MessageGroupId("tx-async"));
    // [BEGIN topic_write_tx]
    ThrowOnError(queryClient.RetryQuerySync([&](NYdb::NQuery::TSession querySession) -> NYdb::TStatus {
        auto beginResult = querySession.BeginTransaction(NYdb::NQuery::TTxSettings::SerializableRW()).GetValueSync();
        if (!beginResult.IsSuccess()) {
            return beginResult;
        }
        auto tx = beginResult.GetTransaction();
        session->Write(NextToken(session), TWriteMessage("tx-async"), &tx);
        Require(session->Flush().GetValueSync(), "Transactional writer did not flush");
        return tx.Commit().GetValueSync();
    }));
    // [END topic_write_tx]
    Require(session->Close(TDuration::Seconds(30)), "Transactional writer did not close");
    auto blocking = client.CreateSimpleBlockingWriteSession(TWriteSessionSettings().Path(path)
        .ProducerId("tx-blocking").MessageGroupId("tx-blocking"));
    // [BEGIN topic_write_tx_blocking]
    ThrowOnError(queryClient.RetryQuerySync([&](NYdb::NQuery::TSession querySession) -> NYdb::TStatus {
        auto beginResult = querySession.BeginTransaction(NYdb::NQuery::TTxSettings::SerializableRW()).GetValueSync();
        if (!beginResult.IsSuccess()) {
            return beginResult;
        }
        auto tx = beginResult.GetTransaction();
        Require(blocking->Write(TWriteMessage("tx-blocking"), &tx, TDuration::Seconds(30)), "Write failed");
        return tx.Commit().GetValueSync();
    }));
    // [END topic_write_tx_blocking]
    Require(blocking->Close(TDuration::Seconds(30)), "Transactional writer did not close");
    const std::map<std::string, size_t> expected = {{"tx-async", 1}, {"tx-blocking", 1}};
    for (const auto& consumer : {"transaction", "deferred_stop"}) {
        auto readSession = client.CreateReadSession(TReadSessionSettings().ConsumerName(consumer).AppendTopics(path));
        std::map<std::string, size_t> received;
        // [BEGIN topic_read_tx]
        ThrowOnError(queryClient.RetryQuerySync([&](NYdb::NQuery::TSession querySession) -> NYdb::TStatus {
            auto beginResult = querySession.BeginTransaction(NYdb::NQuery::TTxSettings::SerializableRW()).GetValueSync();
            if (!beginResult.IsSuccess()) {
                return beginResult;
            }
            auto tx = beginResult.GetTransaction();
            std::vector<TReadSessionEvent::TStopPartitionSessionEvent> stoppedPartitions;
            while (received.size() < expected.size()) {
                auto events = readSession->GetEvents(TReadSessionGetEventSettings().Block(true).Tx(tx));
                for (auto& event : events) {
                    if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&event)) {
                        Record(*data, received, expected);
                    } else if (auto* stop = std::get_if<TReadSessionEvent::TStopPartitionSessionEvent>(&event)) {
                        stoppedPartitions.push_back(std::move(*stop));
                    } else {
                        HandleControl(event);
                    }
                }
            }
            return CommitAndConfirm(tx, stoppedPartitions);
        }));
        // [END topic_read_tx]
        Require(received == expected, "Unexpected transactional message counts");
        Require(readSession->Close(TDuration::Seconds(30)), "Transactional reader did not close");
    }
    ThrowOnError(client.DropTopic(path).GetValueSync());
}

void Run() {
    // [BEGIN topic_init]
    auto config = NYdb::TDriverConfig().SetEndpoint(Environment("YDB_ENDPOINT", "localhost:2136"))
        .SetDatabase(Environment("YDB_DATABASE", "/local"));
    NYdb::TDriver driver(config);
    // [END topic_init]
    // [BEGIN topic_client]
    TTopicClient client(driver);
    // [END topic_client]
    NYdb::NQuery::TQueryClient queryClient(driver);
    const std::string path = "ydb_tech_" + std::to_string(TInstant::Now().MicroSeconds());
    // [BEGIN topic_create]
    auto settings = TCreateTopicSettings().PartitioningSettings(3, 3)
        .AppendSupportedCodecs(ECodec::RAW).AppendSupportedCodecs(ECodec::GZIP).AppendSupportedCodecs(ECodec::ZSTD);
    for (const auto& name : {"poll", "batch", "commit_batch", "offset", "selectors", "autoscale", "outside"}) {
        settings.BeginAddConsumer(name).EndAddConsumer();
    }
    ThrowOnError(client.CreateTopic(path, settings).GetValueSync());
    // [END topic_create]
    try {
        // [BEGIN topic_alter]
        ThrowOnError(client.AlterTopic(path, TAlterTopicSettings().BeginAddConsumer("another-consumer").SetImportant(true)
            .EndAddConsumer().SetRetentionPeriod(TDuration::Days(2))).GetValueSync());
        // [END topic_alter]
        // [BEGIN topic_describe]
        auto description = client.DescribeTopic(path).GetValueSync();
        ThrowOnError(description);
        std::cout << "Consumers: " << description.GetTopicDescription().GetConsumers().size() << std::endl;
        // [END topic_describe]
        Require(description.GetTopicDescription().GetConsumers().size() == 8, "Unexpected consumer count");
        Write(client, path);
        Codecs(client, path);
        Read(client, path, "poll");
        Read(client, path, "offset", true);
        Read(client, path, "", false, true);
        ReadCallbacks(client, path, "batch", false);
        ReadCallbacks(client, path, "commit_batch", true);
        ReadSelectors(client, path);
        Autoscaling(client, path);
        CommitOutside(client, path);
        Transactions(client, queryClient, path + "_tx");
    } catch (...) {
        client.DropTopic(path).GetValueSync();
        throw;
    }
    // [BEGIN topic_drop]
    ThrowOnError(client.DropTopic(path).GetValueSync());
    // [END topic_drop]
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
