#pragma once

#include <ydb-cpp-sdk/client/types/fwd.h>

#include <ydb-cpp-sdk/client/types/exceptions/exceptions.h>
#include <ydb-cpp-sdk/client/types/fatal_error_handlers/handlers.h>
#include <ydb-cpp-sdk/client/types/ydb.h>

#include <ydb-cpp-sdk/library/issue/yql_issue.h>

#include <library/cpp/threading/future/future.h>

#include <utility>

namespace NYdb::inline V3 {

//! Internal status representation
struct TPlainStatus;

//! Represents status of call
class TStatus {
public:
    TStatus(EStatus statusCode, NYdb::NIssue::TIssues&& issues);
    TStatus(TPlainStatus&& plain);

    EStatus GetStatus() const;
    const NYdb::NIssue::TIssues& GetIssues() const;
    bool IsSuccess() const;
    bool IsTransportError() const;
    const std::string& GetEndpoint() const;
    const std::multimap<std::string, std::string>& GetResponseMetadata() const;
    float GetConsumedRu() const;

    void Out(IOutputStream& out) const;
    friend IOutputStream& operator<<(IOutputStream& out, const TStatus& st);

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

namespace NStatusHelpers {

class TYdbErrorException : public TYdbException {
public:
    TYdbErrorException(TStatus status)
        : Status_(std::move(status))
    {
    }

    friend IOutputStream& operator<<(IOutputStream& out, const TYdbErrorException& e) {
        return out << e.Status_;
    }

    const TStatus& GetStatus() const {
        return Status_;
    }

    TStatus&& ExtractStatus() {
        return std::move(Status_);
    }

private:
    TStatus Status_;
};

void ThrowOnError(TStatus status, std::function<void(TStatus)> onSuccess = [](TStatus) {});
void ThrowOnErrorOrPrintIssues(TStatus status);

//! Depth-first search in status issues (including sub-issues). @p pred is invoked on each issue until a match.
template <typename TIssuePredicate>
bool StatusContainsIssueIf(const TStatus& status, TIssuePredicate&& pred) {
    for (const auto& top : status.GetIssues()) {
        bool found = false;
        NYdb::NIssue::WalkThroughIssues(top, false, [&](const NYdb::NIssue::TIssue& issue, uint16_t /*level*/) {
            if (found) {
                return;
            }
            if (std::forward<TIssuePredicate>(pred)(issue)) {
                found = true;
            }
        });
        if (found) {
            return true;
        }
    }
    return false;
}

inline bool StatusContainsIssueWithCode(const TStatus& status, NYdb::NIssue::TIssueCode code) {
    return StatusContainsIssueIf(status, [code](const NYdb::NIssue::TIssue& issue) noexcept {
        return issue.GetCode() == code;
    });
}

}

} // namespace NYdb
