#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>
#include <src/client/persqueue_public/persqueue.h>

namespace NKikimr::NPersQueueTests {

    std::shared_ptr<NYdb::NPersQueue::IWriteSession> CreateWriter(
        NYdb::TDriver& driver,
        const NYdb::NPersQueue::TWriteSessionSettings& settings,
        std::shared_ptr<NYdb::ICredentialsProviderFactory> creds = nullptr
    );

    std::shared_ptr<NYdb::NPersQueue::IWriteSession> CreateWriter(
        NYdb::TDriver& driver,
        const TString& topic,
        const TString& sourceId,
        std::optional<ui32> partitionGroup = {},
        std::optional<TString> codec = {},
        std::optional<bool> reconnectOnFailure = {},
        std::shared_ptr<NYdb::ICredentialsProviderFactory> creds = nullptr
    );

    std::shared_ptr<NYdb::NPersQueue::ISimpleBlockingWriteSession> CreateSimpleWriter(
        NYdb::TDriver& driver,
        const NYdb::NPersQueue::TWriteSessionSettings& settings
    );

    std::shared_ptr<NYdb::NPersQueue::ISimpleBlockingWriteSession> CreateSimpleWriter(
        NYdb::TDriver& driver,
        const TString& topic,
        const TString& sourceId,
        std::optional<ui32> partitionGroup = {},
        std::optional<TString> codec = {},
        std::optional<bool> reconnectOnFailure = {},
        THashMap<TString, TString> sessionMeta = {}
    );

    std::shared_ptr<NYdb::NPersQueue::IReadSession> CreateReader(
        NYdb::TDriver& driver,
        const NYdb::NPersQueue::TReadSessionSettings& settings,
        std::shared_ptr<NYdb::ICredentialsProviderFactory> creds = nullptr

    );

    TMaybe<NYdb::NPersQueue::TReadSessionEvent::TDataReceivedEvent> GetNextMessageSkipAssignment(std::shared_ptr<NYdb::NPersQueue::IReadSession>& reader, TDuration timeout = TDuration::Max());

}
