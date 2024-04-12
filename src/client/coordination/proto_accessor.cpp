<<<<<<<< HEAD:src/client/coordination/proto_accessor.cpp
#include <ydb-cpp-sdk/client/proto/accessor.h>
========
#include <src/client/ydb_proto/accessor.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_coordination/proto_accessor.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_coordination/proto_accessor.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <ydb-cpp-sdk/client/coordination/coordination.h>

namespace NYdb {

const Ydb::Coordination::DescribeNodeResult& TProtoAccessor::GetProto(const NCoordination::TNodeDescription& nodeDescription) {
    return nodeDescription.GetProto();
}

}