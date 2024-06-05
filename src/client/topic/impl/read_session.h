#pragma once

#include <src/client/topic/impl/read_session_impl.ipp>
#include <src/client/topic/impl/callback_context.h>
#include <src/client/topic/impl/counters_logger.h>
#include <src/client/topic/impl/topic_impl.h>

namespace NYdb::NTopic {

class TReadSession : public IReadSession {
public:
    TReadSession(const TReadSessionSettings& settings,
                 std::shared_ptr<TTopicClient::TImpl> client,
                 std::shared_ptr<TGRpcConnectionsImpl> connections,
                 TDbDriverStatePtr dbDriverState);

    ~TReadSession();

    void Start();

    NThreading::TFuture<void> WaitEvent() override;
    std::vector<TReadSessionEvent::TEvent> GetEvents(bool block,
                                                 std::optional<size_t> maxEventsCount,
                                                 size_t maxByteSize) override;
    std::vector<TReadSessionEvent::TEvent> GetEvents(const TReadSessionGetEventSettings& settings) override;
    std::optional<TReadSessionEvent::TEvent> GetEvent(bool block,
                                               size_t maxByteSize) override;
    std::optional<TReadSessionEvent::TEvent> GetEvent(const TReadSessionGetEventSettings& settings) override;

    bool Close(TDuration timeout) override;

    inline std::string GetSessionId() const override {
        return SessionId;
    }

    inline TReaderCounters::TPtr GetCounters() const override {
        return Settings.Counters_; // Always not nullptr.
    }

    void Abort(TSessionClosedEvent&& closeEvent);

    void ClearAllEvents();

private:
    TStringBuilder GetLogPrefix() const;

    // Start
    bool ValidateSettings();

    void CreateClusterSessionsImpl(TDeferredActions<false>& deferred);

    void MakeCountersIfNeeded();
    void SetupCountersLogger();

    // Shutdown.
    void Abort(EStatus statusCode, NYql::TIssues&& issues);
    void Abort(EStatus statusCode, const std::string& message);

    void AbortImpl(TDeferredActions<false>& deferred);
    void AbortImpl(TSessionClosedEvent&& closeEvent, TDeferredActions<false>& deferred);
    void AbortImpl(EStatus statusCode, NYql::TIssues&& issues, TDeferredActions<false>& deferred);
    void AbortImpl(EStatus statusCode, const std::string& message, TDeferredActions<false>& deferred);

private:
    using TOffsetRanges = THashMap<std::string, THashMap<ui64, TDisjointIntervalTree<ui64>>>;

    void CollectOffsets(NTable::TTransaction& tx,
                        const TReadSessionEvent::TDataReceivedEvent& event);
    void CollectOffsets(NTable::TTransaction& tx,
                        const std::string& topicPath, ui32 partitionId, ui64 offset);
    void UpdateOffsets(const NTable::TTransaction& tx);

    //
    // (session, tx) -> topic -> partition -> (begin, end)
    //
    THashMap<std::pair<std::string, std::string>, TOffsetRanges> OffsetRanges;

    TReadSessionSettings Settings;
    const std::string SessionId;
    const TInstant StartSessionTime = TInstant::Now();
    TLog Log;
    std::shared_ptr<TTopicClient::TImpl> Client;
    std::shared_ptr<TGRpcConnectionsImpl> Connections;
    TDbDriverStatePtr DbDriverState;
    TAdaptiveLock Lock;
    std::shared_ptr<TReadSessionEventsQueue<false>> EventsQueue;

    std::shared_ptr<TCallbackContext<TSingleClusterReadSessionImpl<false>>> CbContext;
    std::vector<TTopicReadSettings> Topics;

    std::shared_ptr<TCountersLogger<false>> CountersLogger;
    std::shared_ptr<TCallbackContext<TCountersLogger<false>>> DumpCountersContext;

    // Exiting.
    bool Aborting = false;
    bool Closing = false;
};

} // namespace NYdb::NTopic
