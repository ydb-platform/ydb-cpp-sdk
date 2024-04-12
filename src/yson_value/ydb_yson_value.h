#pragma once

#include <src/client/ydb_result/result.h>
#include <src/client/ydb_value/value.h>
#include <src/client/ydb_types/fatal_error_handlers/handlers.h>

#include <src/library/yson/node/node_io.h>

#include <src/library/yson/writer.h>

namespace NYdb {

void FormatValueYson(const TValue& value, NYson::TYsonWriter& writer);

std::string FormatValueYson(const TValue& value, NYson::EYsonFormat ysonFormat = NYson::EYsonFormat::Text);

void FormatResultSetYson(const TResultSet& result, NYson::TYsonWriter& writer);

std::string FormatResultSetYson(const TResultSet& result, NYson::EYsonFormat ysonFormat = NYson::EYsonFormat::Text);

} // namespace NYdb
