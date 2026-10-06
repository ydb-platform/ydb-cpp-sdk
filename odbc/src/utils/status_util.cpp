#include "status_util.h"

#include <ydb-cpp-sdk/library/issue/yql_issue.h>

namespace NYdb::NOdbc {

NYdb::TStatus StatusFrom(const NYdb::TStatus& ydbStatus) {
    return NYdb::TStatus(ydbStatus.GetStatus(), NYdb::NIssue::TIssues(ydbStatus.GetIssues()));
}

bool IsSessionInvalidated(const NYdb::TStatus& status) {
    // Match the SDK's session-status interception, not DeleteSession's acknowledgement.
    return status.GetStatus() == EStatus::BAD_SESSION
        || status.GetStatus() == EStatus::SESSION_BUSY
        || (status.IsTransportError()
            && status.GetStatus() != EStatus::CLIENT_RESOURCE_EXHAUSTED
            && status.GetStatus() != EStatus::CLIENT_OUT_OF_RANGE);
}

} // namespace NYdb::NOdbc
