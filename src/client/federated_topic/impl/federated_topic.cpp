<<<<<<< HEAD
<<<<<<<< HEAD:src/client/federated_topic/impl/federated_topic.cpp
#include <ydb-cpp-sdk/client/federated_topic/federated_topic.h>
#include <src/client/federated_topic/impl/federated_topic_impl.h>
========
#include <src/client/ydb_federated_topic/federated_topic.h>
#include <src/client/ydb_federated_topic/impl/federated_topic_impl.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_federated_topic/impl/federated_topic.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_federated_topic/impl/federated_topic.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/federated_topic/federated_topic.h>
#include <src/client/federated_topic/impl/federated_topic_impl.h>
>>>>>>> origin/main

namespace NYdb::NFederatedTopic {

NTopic::TTopicClientSettings FromFederated(const TFederatedTopicClientSettings& settings) {
    return NTopic::TTopicClientSettings()
        .DefaultCompressionExecutor(settings.DefaultCompressionExecutor_)
        .DefaultHandlersExecutor(settings.DefaultHandlersExecutor_);
}

TFederatedTopicClient::TFederatedTopicClient(const TDriver& driver, const TFederatedTopicClientSettings& settings)
    : Impl_(std::make_shared<TImpl>(CreateInternalInterface(driver), settings))
{
}

std::shared_ptr<IFederatedReadSession> TFederatedTopicClient::CreateReadSession(const TFederatedReadSessionSettings& settings) {
    return Impl_->CreateReadSession(settings);
}

// std::shared_ptr<NTopic::ISimpleBlockingWriteSession> TFederatedTopicClient::CreateSimpleBlockingWriteSession(
//     const TFederatedWriteSessionSettings& settings) {
//     return Impl_->CreateSimpleBlockingWriteSession(settings);
// }

std::shared_ptr<NTopic::IWriteSession> TFederatedTopicClient::CreateWriteSession(const TFederatedWriteSessionSettings& settings) {
    return Impl_->CreateWriteSession(settings);
}

} // namespace NYdb::NFederatedTopic
