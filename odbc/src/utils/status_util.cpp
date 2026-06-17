#include "status_util.h"

#include <ydb-cpp-sdk/library/issue/yql_issue.h>

namespace NYdb::NOdbc {

NYdb::TStatus StatusFrom(const NYdb::TStatus& ydbStatus) {
    return NYdb::TStatus(ydbStatus.GetStatus(), NYdb::NIssue::TIssues(ydbStatus.GetIssues()));
}

} // namespace NYdb::NOdbc
