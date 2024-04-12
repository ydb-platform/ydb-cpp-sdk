#include "federated_topic_impl.h"

#include "federated_read_session.h"
#include "federated_write_session.h"

<<<<<<<< HEAD:src/client/federated_topic/impl/federated_topic_impl.cpp
#include <src/client/persqueue_core/impl/read_session.h>
#include <src/client/persqueue_core/impl/write_session.h>
========
#include <src/client/ydb_persqueue_core/impl/read_session.h>
#include <src/client/ydb_persqueue_core/impl/write_session.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_federated_topic/impl/federated_topic_impl.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_federated_topic/impl/federated_topic_impl.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb::NFederatedTopic {

std::shared_ptr<IFederatedReadSession>
TFederatedTopicClient::TImpl::CreateReadSession(const TFederatedReadSessionSettings& settings) {
    InitObserver();
    auto session = std::make_shared<TFederatedReadSession>(settings, Connections, ClientSettings, GetObserver());
    session->Start();
    return std::move(session);
}

// std::shared_ptr<NTopic::ISimpleBlockingWriteSession>
// TFederatedTopicClient::TImpl::CreateSimpleBlockingWriteSession(const TFederatedWriteSessionSettings& settings) {
//     InitObserver();
//     auto session = std::make_shared<TSimpleBlockingFederatedWriteSession>(settings, Connections, ClientSettings, GetObserver());
//     session->Start();
//     return std::move(session);

// }

std::shared_ptr<NTopic::IWriteSession>
TFederatedTopicClient::TImpl::CreateWriteSession(const TFederatedWriteSessionSettings& settings) {
    // Split settings.MaxMemoryUsage_ by two.
    // One half goes to subsession. Other half goes to federated session internal buffer.
    const ui64 splitSize = (settings.MaxMemoryUsage_ + 1) / 2;
    TFederatedWriteSessionSettings splitSettings = settings;
    splitSettings.MaxMemoryUsage(splitSize);
    InitObserver();
    auto session = std::make_shared<TFederatedWriteSession>(splitSettings, Connections, ClientSettings, GetObserver());
    session->Start();
    return std::move(session);

}

void TFederatedTopicClient::TImpl::InitObserver() {
    std::lock_guard guard(Lock);
    if (!Observer || Observer->IsStale()) {
        Observer = std::make_shared<TFederatedDbObserver>(Connections, ClientSettings);
        Observer->Start();
    }
}

}
