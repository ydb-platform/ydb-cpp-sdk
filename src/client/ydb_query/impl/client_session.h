#pragma once

<<<<<<<< HEAD:src/client/query/impl/client_session.h
#include <ydb-cpp-sdk/client/query/client.h>
========
#include <src/client/ydb_query/client.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_query/impl/client_session.h
#include <src/client/impl/ydb_internal/kqp_session_common/kqp_session_common.h>

namespace NYdb::NQuery {

class TSession::TImpl : public TKqpSessionCommon {
public:
    struct TAttachSessionArgs {
        TAttachSessionArgs(NThreading::TPromise<TCreateSessionResult> promise,
            std::string sessionId,
            std::string endpoint,
            std::shared_ptr<TQueryClient::TImpl> client)
            : Promise(promise)
            , SessionId(sessionId)
            , Endpoint(endpoint)
            , Client(client)
        { }
        NThreading::TPromise<TCreateSessionResult> Promise;
        std::string SessionId;
        std::string Endpoint;
        std::shared_ptr<TQueryClient::TImpl> Client;
    };

    using TResponse = Ydb::Query::SessionState;
    using TStreamProcessorPtr = NYdbGrpc::IStreamRequestReadProcessor<TResponse>::TPtr;
    TImpl(TStreamProcessorPtr ptr, const std::string& id, const std::string& endpoint);
    ~TImpl();

    static void MakeImplAsync(TStreamProcessorPtr processor, std::shared_ptr<TAttachSessionArgs> args);

private:
    static void NewSmartShared(TStreamProcessorPtr ptr, std::shared_ptr<TAttachSessionArgs> args, NYdb::TStatus status);

private:
    TStreamProcessorPtr StreamProcessor_;
};

}
