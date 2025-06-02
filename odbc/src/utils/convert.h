#pragma once

#include "bindings.h"

#include <ydb-cpp-sdk/client/params/params.h>

#include <sql.h>
#include <sqlext.h>

namespace NYdb {
namespace NOdbc {

SQLRETURN ConvertParam(const TBoundParam& param, TParamValueBuilder& builder);
SQLRETURN ConvertColumn(TValueParser& parser, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);

} // namespace NYdb
} // namespace NOdbc
