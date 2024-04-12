#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/types/status/status.h
#include <ydb-cpp-sdk/client/types/fatal_error_handlers/handlers.h>
#include <ydb-cpp-sdk/client/types/ydb.h>

#include <ydb-cpp-sdk/library/yql/public/issue/yql_issue.h>

#include <ydb-cpp-sdk/library/threading/future/future.h>
========
#include <src/client/ydb_types/fatal_error_handlers/handlers.h>
#include <src/client/ydb_types/ydb.h>

#include <src/library/yql/public/issue/yql_issue.h>

#include <src/library/threading/future/future.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/status/status.h

namespace NYdb {

//! Internal status representation
struct TPlainStatus;

//! Represents status of call
class TStatus {
public:
    TStatus(EStatus statusCode, NYql::TIssues&& issues);
    TStatus(TPlainStatus&& plain);

    EStatus GetStatus() const;
    const NYql::TIssues& GetIssues() const;
    bool IsSuccess() const;
    bool IsTransportError() const;
    const std::string& GetEndpoint() const;
    const std::multimap<std::string, std::string>& GetResponseMetadata() const;
    float GetConsumedRu() const;

    friend IOutputStream& operator<<(IOutputStream& out, const TStatus& st);
    friend std::ostream& operator<<(std::ostream& out, const TStatus& st);

protected:
    void CheckStatusOk(const std::string& str) const;
    void RaiseError(const std::string& str) const;
private:
    class TImpl;
    std::shared_ptr<TImpl> Impl_;
};

using TAsyncStatus = NThreading::TFuture<TStatus>;

class TStreamPartStatus : public TStatus {
public:
    TStreamPartStatus(TStatus&& status);
    bool EOS() const;
};

} // namespace NYdb
