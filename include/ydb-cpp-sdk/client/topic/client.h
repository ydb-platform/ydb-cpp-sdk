#pragma once

#include "control_plane.h"
#include "read_session.h"
#include "write_session.h"

namespace NYdb::NTopic {

struct TTopicClientSettings : public TCommonClientSettingsBase<TTopicClientSettings> {
    using TSelf = TTopicClientSettings;

    //! Default executor for compression tasks.
    FLUENT_SETTING_DEFAULT(IExecutor::TPtr, DefaultCompressionExecutor, CreateThreadPoolExecutor(2));

    //! Default executor for callbacks.
    FLUENT_SETTING_DEFAULT(IExecutor::TPtr, DefaultHandlersExecutor, CreateThreadPoolExecutor(1));
};

// Topic client.
class TTopicClient {
public:
    class TImpl;

    TTopicClient(const TDriver& driver, const TTopicClientSettings& settings = TTopicClientSettings());

    void ProvideCodec(ECodec codecId, THolder<ICodec>&& codecImpl);

    // Create a new topic.
    TAsyncStatus CreateTopic(const std::string& path, const TCreateTopicSettings& settings = {});

    // Update a topic.
    TAsyncStatus AlterTopic(const std::string& path, const TAlterTopicSettings& settings = {});

    // Delete a topic.
    TAsyncStatus DropTopic(const std::string& path, const TDropTopicSettings& settings = {});

    // Describe a topic.
    TAsyncDescribeTopicResult DescribeTopic(const std::string& path, const TDescribeTopicSettings& settings = {});

    // Describe a topic consumer.
    TAsyncDescribeConsumerResult DescribeConsumer(const std::string& path, const std::string& consumer, const TDescribeConsumerSettings& settings = {});

    // Describe a topic partition
    TAsyncDescribePartitionResult DescribePartition(const std::string& path, i64 partitionId, const TDescribePartitionSettings& settings = {});

    //! Create read session.
    std::shared_ptr<IReadSession> CreateReadSession(const TReadSessionSettings& settings);

    //! Create write session.
    std::shared_ptr<ISimpleBlockingWriteSession> CreateSimpleBlockingWriteSession(const TWriteSessionSettings& settings);
    std::shared_ptr<IWriteSession> CreateWriteSession(const TWriteSessionSettings& settings);

    // Commit offset
    TAsyncStatus CommitOffset(const std::string& path, ui64 partitionId, const std::string& consumerName, ui64 offset,
        const TCommitOffsetSettings& settings = {});

protected:
    void OverrideCodec(ECodec codecId, THolder<ICodec>&& codecImpl);

private:
    std::shared_ptr<TImpl> Impl_;
};

} // namespace NYdb::NTopic
