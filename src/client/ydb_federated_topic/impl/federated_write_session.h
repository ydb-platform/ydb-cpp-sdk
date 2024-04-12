#pragma once

#include <src/client/ydb_federated_topic/impl/federated_topic_impl.h>

#include <src/client/ydb_persqueue_core/impl/write_session.h>

#include <src/client/ydb_topic/impl/write_session.h>

#include <deque>

namespace NYdb::NFederatedTopic {

class TFederatedWriteSession : public NTopic::IWriteSession,
                               public NTopic::TContinuationTokenIssuer,
                               public std::enable_shared_from_this<TFederatedWriteSession> {
    friend class TFederatedTopicClient::TImpl;

public:
    struct TDeferredWrite {
        explicit TDeferredWrite(std::shared_ptr<NTopic::IWriteSession> writer)
            : Writer(std::move(writer)) {
        }

        void DoWrite() {
            if (!Token.has_value() && !Message.has_value()) {
                return;
            }
            Y_ABORT_UNLESS(Token.has_value() && Message.has_value());
            return Writer->Write(std::move(*Token), std::move(*Message));
        }

        std::shared_ptr<NTopic::IWriteSession> Writer;
        std::optional<NTopic::TContinuationToken> Token;
        std::optional<NTopic::TWriteMessage> Message;
    };

    TFederatedWriteSession(const TFederatedWriteSessionSettings& settings,
                          std::shared_ptr<TGRpcConnectionsImpl> connections,
                          const TFederatedTopicClientSettings& clientSetttings,
                          std::shared_ptr<TFederatedDbObserver> observer);

    ~TFederatedWriteSession() = default;

    NThreading::TFuture<void> WaitEvent() override;
    std::optional<NTopic::TWriteSessionEvent::TEvent> GetEvent(bool block) override;
    std::vector<NTopic::TWriteSessionEvent::TEvent> GetEvents(bool block, std::optional<size_t> maxEventsCount) override;

    virtual NThreading::TFuture<ui64> GetInitSeqNo() override;

    virtual void Write(NTopic::TContinuationToken&& continuationToken, NTopic::TWriteMessage&& message) override;

    virtual void WriteEncoded(NTopic::TContinuationToken&& continuationToken, NTopic::TWriteMessage&& params) override;

    virtual void Write(NTopic::TContinuationToken&&, std::string_view, std::optional<ui64> seqNo = std::nullopt,
                       std::optional<TInstant> createTimestamp = std::nullopt) override;

    virtual void WriteEncoded(NTopic::TContinuationToken&&, std::string_view, NTopic::ECodec, ui32,
                              std::optional<ui64> seqNo = std::nullopt, std::optional<TInstant> createTimestamp = std::nullopt) override;

    bool Close(TDuration timeout) override;

    inline NTopic::TWriterCounters::TPtr GetCounters() override {Y_ABORT("Unimplemented"); } //ToDo - unimplemented;

private:
    void Start();
    void OpenSubSessionImpl(std::shared_ptr<TDbInfo> db);

    std::shared_ptr<TDbInfo> SelectDatabaseImpl();

    void OnFederatedStateUpdateImpl();
    void ScheduleFederatedStateUpdateImpl(TDuration delay);

    void WriteInternal(NTopic::TContinuationToken&&, NTopic::TWriteMessage&& message);
    bool PrepareDeferredWrite(TDeferredWrite& deferred);

    void CloseImpl(EStatus statusCode, NYql::TIssues&& issues);

private:
    // For subsession creation
    const NTopic::TFederatedWriteSessionSettings Settings;
    std::shared_ptr<TGRpcConnectionsImpl> Connections;
    const NTopic::TTopicClientSettings SubClientSetttings;

    std::shared_ptr<TFederatedDbObserver> Observer;
    NThreading::TFuture<void> AsyncInit;
    std::shared_ptr<TFederatedDbState> FederationState;
    NYdbGrpc::IQueueClientContextPtr UpdateStateDelayContext;

    std::shared_ptr<TDbInfo> CurrentDatabase;

    std::string SessionId;
    const TInstant StartSessionTime = TInstant::Now();

    TAdaptiveLock Lock;

    std::shared_ptr<NTopic::IWriteSession> Subsession;

    std::shared_ptr<NTopic::TWriteSessionEventsQueue> ClientEventsQueue;

    std::optional<NTopic::TContinuationToken> PendingToken;  // from Subsession
    bool ClientHasToken = false;
    std::deque<NTopic::TWriteMessage> OriginalMessagesToPassDown;
    std::deque<NTopic::TWriteMessage> OriginalMessagesToGetAck;
    i64 BufferFreeSpace;

    // Exiting.
    bool Closing = false;
};

} // namespace NYdb::NFederatedTopic
