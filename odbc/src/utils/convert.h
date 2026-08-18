#pragma once

#include "bindings.h"

#include <ydb-cpp-sdk/client/params/params.h>

#include <sql.h>
#include <sqlext.h>

#include <string>
#include <variant>

namespace NYdb::NOdbc {

using TOdbcScalar = std::variant<std::monostate, int64_t, uint64_t, double, std::string>;

SQLRETURN ConvertParam(const TBoundParam& param, TParamValueBuilder& builder);
SQLRETURN ConvertColumn(const TOdbcScalar& value, SQLSMALLINT targetType, SQLPOINTER targetValue,
                        SQLLEN bufferLength, SQLLEN* strLenOrInd, SQLLEN* offset = nullptr);
SQLRETURN ConvertColumn(TValueParser& parser, SQLSMALLINT targetType, SQLPOINTER targetValue,
                        SQLLEN bufferLength, SQLLEN* strLenOrInd, SQLLEN* offset = nullptr);
const char* ConsumeLastConvertSqlState();

} // namespace NYdb::NOdbc
