#pragma once

#include <src/client/impl/ydb_endpoints/endpoints.h>
#include <src/client/impl/ydb_internal/session_client/session_client.h>

#include <ydb-cpp-sdk/util/datetime/base.h>
#include <ydb-cpp-sdk/util/system/spinlock.h>

#include <functional>

namespace NYdb {

////////////////////////////////////////////////////////////////////////////////
ui64 GetNodeIdFromSession(const std::string& sessionId);

class TKqpSessionCommon : public TEndpointObj {
public:
    TKqpSessionCommon(const std::string& sessionId, const std::string& endpoint,
        bool isOwnedBySessionPool);

    enum EState {
        S_STANDALONE,
        S_IDLE,
        S_BROKEN,
        S_ACTIVE,
        S_CLOSING
    };

public:
    ~TKqpSessionCommon();

    const std::string& GetId() const;
    const std::string& GetEndpoint() const;
    const TEndpointKey& GetEndpointKey() const;
    void MarkBroken();
    void MarkAsClosing();
    void MarkActive();
    void MarkIdle();
    bool IsOwnedBySessionPool() const;
    EState GetState() const;
    void SetNeedUpdateActiveCounter(bool flag);
    bool NeedUpdateActiveCounter() const;
    void InvalidateQueryInCache(const std::string& key);
    void InvalidateQueryCache();
    void ScheduleTimeToTouch(TDuration interval, bool updateTimeInPast);
    void ScheduleTimeToTouchFast(TDuration interval, bool updateTimeInPast);
    TInstant GetTimeToTouchFast() const;
    TInstant GetTimeInPastFast() const;

    // SetTimeInterval/GetTimeInterval, are not atomic!
    void SetTimeInterval(TDuration interval);
    TDuration GetTimeInterval() const;

    static std::function<void(TKqpSessionCommon*)>
        GetSmartDeleter(std::shared_ptr<ISessionClient> client);

protected:
    TAdaptiveLock Lock_;

private:
    const std::string SessionId_;
    const TEndpointKey EndpointKey_;
    const bool IsOwnedBySessionPool_;

    EState State_;
    TInstant TimeToTouch_;
    TInstant TimeInPast_;
    // Is used to implement progressive timeout for settler keep alive call
    TDuration TimeInterval_;
    // Indicate session was in active state, but state was changed
    // (need to decrement active session counter)
    // TODO: suboptimal because need lock for atomic change from interceptor
    // Rewrite with bit field
    bool NeedUpdateActiveCounter_;
};

} // namespace NYdb
