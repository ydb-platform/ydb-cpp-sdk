#pragma once

#include <client/impl/ydb_internal/internal_header.h>

#include <ydb/public/api/protos/ydb_value.pb.h>

namespace NYdb {

bool TypesEqual(const Ydb::Type& t1, const Ydb::Type& t2);

} // namespace NYdb
