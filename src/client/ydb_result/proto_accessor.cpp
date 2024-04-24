#include "result.h"

#include <src/client/ydb_proto/accessor.h>

namespace NYdb {

const Ydb::ResultSet& TProtoAccessor::GetProto(const TResultSet& resultSet) {
    return resultSet.GetProto();
}

} // namespace NYdb
