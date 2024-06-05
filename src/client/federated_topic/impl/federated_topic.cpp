#include <ydb-cpp-sdk/client/federated_topic/federated_topic.h>
#include <src/client/federated_topic/impl/federated_topic_impl.h>

namespace NYdb::NFederatedTopic {

// TFederatedReadSessionSettings
// Read policy settings

using TReadOriginalSettings = TFederatedReadSessionSettings::TReadOriginalSettings;
TReadOriginalSettings& TReadOriginalSettings::AddDatabase(const std::string& database) {
    Databases.insert(std::move(database));
    return *this;
}

TReadOriginalSettings& TReadOriginalSettings::AddDatabases(const std::vector<std::string>& databases) {
    std::move(std::begin(databases), std::end(databases), std::inserter(Databases, Databases.end()));
    return *this;
}

TReadOriginalSettings& TReadOriginalSettings::AddLocal() {
    Databases.insert("_local");
    return *this;
}

TFederatedReadSessionSettings& TFederatedReadSessionSettings::ReadOriginal(TReadOriginalSettings settings) {
    std::swap(DatabasesToReadFrom, settings.Databases);
    ReadMirroredEnabled = false;
    return *this;
}

TFederatedReadSessionSettings& TFederatedReadSessionSettings::ReadMirrored(const std::string& database) {
    if (database == "_local") {
        ythrow TContractViolation("Reading from local database not supported, use specific database");
    }
    DatabasesToReadFrom.clear();
    DatabasesToReadFrom.insert(std::move(database));
    ReadMirroredEnabled = true;
    return *this;
}

// TFederatedTopicClient

NTopic::TTopicClientSettings FromFederated(const TFederatedTopicClientSettings& fedSettings) {
    auto settings = NTopic::TTopicClientSettings()
        .DefaultCompressionExecutor(fedSettings.DefaultCompressionExecutor_)
        .DefaultHandlersExecutor(fedSettings.DefaultHandlersExecutor_);

    if (fedSettings.CredentialsProviderFactory_) {
        settings.CredentialsProviderFactory(*fedSettings.CredentialsProviderFactory_);
    }
    if (fedSettings.SslCredentials_) {
        settings.SslCredentials(*fedSettings.SslCredentials_);
    }
    if (fedSettings.DiscoveryMode_) {
        settings.DiscoveryMode(*fedSettings.DiscoveryMode_);
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
