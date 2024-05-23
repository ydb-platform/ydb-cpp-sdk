#include <ydb-cpp-sdk/client/federated_topic/federated_topic.h>
#include <src/client/federated_topic/impl/federated_topic_impl.h>

namespace NYdb::NFederatedTopic {

NTopic::TTopicClientSettings FromFederated(const TFederatedTopicClientSettings& fedSettings) {
    auto settings = NTopic::TTopicClientSettings()
        .DefaultCompressionExecutor(fedSettings.DefaultCompressionExecutor_)
        .DefaultHandlersExecutor(fedSettings.DefaultHandlersExecutor_);
    if (fedSettings.CredentialsProviderFactory_) {
        settings.CredentialsProviderFactory(*fedSettings.CredentialsProviderFactory_);
    }
    return settings;
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
