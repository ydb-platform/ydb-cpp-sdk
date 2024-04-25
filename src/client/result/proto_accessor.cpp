#include <ydb-cpp-sdk/client/result/result.h>

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/result/proto_accessor.cpp
#include <ydb-cpp-sdk/client/proto/accessor.h>
========
#include <src/client/ydb_proto/accessor.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_result/proto_accessor.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_result/proto_accessor.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/proto/accessor.h>
>>>>>>> origin/main

namespace NYdb {

const Ydb::ResultSet& TProtoAccessor::GetProto(const TResultSet& resultSet) {
    return resultSet.GetProto();
}

} // namespace NYdb
