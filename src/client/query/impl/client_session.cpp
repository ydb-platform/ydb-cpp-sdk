#include "client_session.h"

#define INCLUDE_YDB_INTERNAL_H
#include <src/client/impl/ydb_internal/plain_status/status.h>
#undef INCLUDE_YDB_INTERNAL_H

#include <src/library/issue/yql_issue_message.h>

namespace NYdb::inline V3::NQuery {

TSession::TImpl::TImpl(TStreamProcessorPtr ptr, const std::string& sessionId, const std::string& endpoint)
    : TKqpSessionCommon(sessionId, endpoint, true)
    , StreamProcessor_(ptr)
{
    MarkActive();
    SetNeedUpdateActiveCounter(true);
}

TSession::TImpl::~TImpl()
{
    StreamProcessor_->Cancel();
}

void TSession::TImpl::MakeImplAsync(TStreamProcessorPtr ptr,
    std::shared_ptr<TAttachSessionArgs> args)
{
    auto resp = std::make_shared<Ydb::Query::SessionState>();
    ptr->Read(resp.get(), [args, resp, ptr](NYdbGrpc::TGrpcStatus grpcStatus) mutable {
        if (grpcStatus.GRpcStatusCode != grpc::StatusCode::OK) {
            TStatus st(TPlainStatus(grpcStatus, args->Endpoint));
            args->Promise.SetValue(TCreateSessionResult(std::move(st), TSession()));

        } else {
            if (resp->status() == Ydb::StatusIds::SUCCESS) {
                NYdb::TStatus st(TPlainStatus(grpcStatus, args->Endpoint));
                TSession::TImpl::NewSmartShared(ptr, std::move(args), st);

            } else {
                NYdb::NIssue::TIssues opIssues;
                NYdb::NIssue::IssuesFromMessage(resp->issues(), opIssues);
                TStatus st(static_cast<EStatus>(resp->status()), std::move(opIssues));
                args->Promise.SetValue(TCreateSessionResult(std::move(st), TSession()));
            }
        }
    });
}

void TSession::TImpl::NewSmartShared(TStreamProcessorPtr ptr,
    std::shared_ptr<TAttachSessionArgs> args, NYdb::TStatus st)
{
    args->Promise.SetValue(
        TCreateSessionResult(
            std::move(st),
            TSession(
                args->Client,
                new TSession::TImpl(ptr, args->SessionId, args->Endpoint)
            )
        )
    );
}

}
