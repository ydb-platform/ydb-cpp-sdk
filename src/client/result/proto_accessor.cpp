#include <ydb-cpp-sdk/client/result/result.h>

#include <src/client/proto/accessor.h>

namespace NYdb {

const Ydb::ResultSet& TProtoAccessor::GetProto(const TResultSet& resultSet) {
    return resultSet.GetProto();
}

} // namespace NYdb
