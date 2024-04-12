#include <ydb-cpp-sdk/client/result/result.h>

<<<<<<<< HEAD:src/client/result/proto_accessor.cpp
#include <ydb-cpp-sdk/client/proto/accessor.h>
========
#include <src/client/ydb_proto/accessor.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_result/proto_accessor.cpp

namespace NYdb {

const Ydb::ResultSet& TProtoAccessor::GetProto(const TResultSet& resultSet) {
    return resultSet.GetProto();
}

} // namespace NYdb
