#include "topic_impl.h"

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/topic/impl/topic_impl.cpp
#include <src/client/persqueue_core/impl/read_session.h>
#include <src/client/persqueue_core/impl/write_session.h>
========
#include <src/client/ydb_persqueue_core/impl/read_session.h>
#include <src/client/ydb_persqueue_core/impl/write_session.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_topic/impl/topic_impl.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_topic/impl/topic_impl.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <src/client/persqueue_core/impl/read_session.h>
#include <src/client/persqueue_core/impl/write_session.h>
>>>>>>> origin/main

#include "read_session.h"
#include "write_session.h"

namespace NYdb::NTopic {

std::shared_ptr<IReadSession> TTopicClient::TImpl::CreateReadSession(const TReadSessionSettings& settings) {
    std::optional<TReadSessionSettings> maybeSettings;
    if (!settings.DecompressionExecutor_ || !settings.EventHandlers_.HandlersExecutor_) {
        maybeSettings = settings;
        std::lock_guard guard(Lock);
        if (!settings.DecompressionExecutor_) {
            maybeSettings->DecompressionExecutor(Settings.DefaultCompressionExecutor_);
        }
        if (!settings.EventHandlers_.HandlersExecutor_) {
            maybeSettings->EventHandlers_.HandlersExecutor(Settings.DefaultHandlersExecutor_);
        }
    }
    auto session = std::make_shared<TReadSession>(maybeSettings.value_or(settings), shared_from_this(), Connections_, DbDriverState_);
    session->Start();
    return std::move(session);
}

std::shared_ptr<IWriteSession> TTopicClient::TImpl::CreateWriteSession(
        const TWriteSessionSettings& settings
) {
    std::optional<TWriteSessionSettings> maybeSettings;
    if (!settings.CompressionExecutor_ || !settings.EventHandlers_.HandlersExecutor_) {
        maybeSettings = settings;
        std::lock_guard guard(Lock);
        if (!settings.CompressionExecutor_) {
            maybeSettings->CompressionExecutor(Settings.DefaultCompressionExecutor_);
        }
        if (!settings.EventHandlers_.HandlersExecutor_) {
            maybeSettings->EventHandlers_.HandlersExecutor(Settings.DefaultHandlersExecutor_);
        }
    }
    auto session = std::make_shared<TWriteSession>(
            maybeSettings.value_or(settings), shared_from_this(), Connections_, DbDriverState_
    );
    session->Start(TDuration::Zero());
    return std::move(session);
}

std::shared_ptr<ISimpleBlockingWriteSession> TTopicClient::TImpl::CreateSimpleWriteSession(
        const TWriteSessionSettings& settings
) {
    auto alteredSettings = settings;
    {
        std::lock_guard guard(Lock);
        alteredSettings.EventHandlers_.HandlersExecutor(Settings.DefaultHandlersExecutor_);
        if (!settings.CompressionExecutor_) {
            alteredSettings.CompressionExecutor(Settings.DefaultCompressionExecutor_);
        }
    }

    auto session = std::make_shared<TSimpleBlockingWriteSession>(
            alteredSettings, shared_from_this(), Connections_, DbDriverState_
    );
    return std::move(session);
}

std::shared_ptr<TTopicClient::TImpl::IReadSessionConnectionProcessorFactory> TTopicClient::TImpl::CreateReadSessionConnectionProcessorFactory() {
    using TService = Ydb::Topic::V1::TopicService;
    using TRequest = Ydb::Topic::StreamReadMessage::FromClient;
    using TResponse = Ydb::Topic::StreamReadMessage::FromServer;
    return NPersQueue::CreateConnectionProcessorFactory<TService, TRequest, TResponse>(&TService::Stub::AsyncStreamRead, Connections_, DbDriverState_);
}

std::shared_ptr<TTopicClient::TImpl::IWriteSessionConnectionProcessorFactory> TTopicClient::TImpl::CreateWriteSessionConnectionProcessorFactory() {
    using TService = Ydb::Topic::V1::TopicService;
    using TRequest = Ydb::Topic::StreamWriteMessage::FromClient;
    using TResponse = Ydb::Topic::StreamWriteMessage::FromServer;
    return NPersQueue::CreateConnectionProcessorFactory<TService, TRequest, TResponse>(&TService::Stub::AsyncStreamWrite, Connections_, DbDriverState_);
}

}
