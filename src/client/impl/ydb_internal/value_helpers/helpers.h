#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

#include <ydb-cpp-sdk/src/api/protos/ydb_value.pb.h>

namespace NYdb {

bool TypesEqual(const Ydb::Type& t1, const Ydb::Type& t2);

} // namespace NYdb
