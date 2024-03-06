#pragma once

#include <client/ydb_driver/driver.h>
#include <client/ydb_table/table.h>
#include <client/ydb_persqueue_public/persqueue.h>

namespace NKikimr::NPersQueueTests {

    std::shared_ptr<NYdb::NPersQueue::IWriteSession> CreateWriter(
        NYdb::TDriver& driver,
        const NYdb::NPersQueue::TWriteSessionSettings& settings,
        std::shared_ptr<NYdb::ICredentialsProviderFactory> creds = nullptr
    );

    std::shared_ptr<NYdb::NPersQueue::IWriteSession> CreateWriter(
        NYdb::TDriver& driver,
        const std::string& topic,
        const std::string& sourceId,
        std::optional<ui32> partitionGroup = {},
        std::optional<std::string> codec = {},
        std::optional<bool> reconnectOnFailure = {},
        std::shared_ptr<NYdb::ICredentialsProviderFactory> creds = nullptr
    );

    std::shared_ptr<NYdb::NPersQueue::ISimpleBlockingWriteSession> CreateSimpleWriter(
        NYdb::TDriver& driver,
        const NYdb::NPersQueue::TWriteSessionSettings& settings
    );

    std::shared_ptr<NYdb::NPersQueue::ISimpleBlockingWriteSession> CreateSimpleWriter(
        NYdb::TDriver& driver,
        const std::string& topic,
        const std::string& sourceId,
        std::optional<ui32> partitionGroup = {},
        std::optional<std::string> codec = {},
        std::optional<bool> reconnectOnFailure = {},
        THashMap<std::string, std::string> sessionMeta = {}
    );

    std::shared_ptr<NYdb::NPersQueue::IReadSession> CreateReader(
        NYdb::TDriver& driver,
        const NYdb::NPersQueue::TReadSessionSettings& settings,
        std::shared_ptr<NYdb::ICredentialsProviderFactory> creds = nullptr

    );

    std::optional<NYdb::NPersQueue::TReadSessionEvent::TDataReceivedEvent> GetNextMessageSkipAssignment(std::shared_ptr<NYdb::NPersQueue::IReadSession>& reader, TDuration timeout = TDuration::Max());

}
