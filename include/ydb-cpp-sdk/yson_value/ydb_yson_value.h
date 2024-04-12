#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/yson_value/ydb_yson_value.h
#include <ydb-cpp-sdk/client/result/result.h>
#include <ydb-cpp-sdk/client/value/value.h>
#include <ydb-cpp-sdk/client/types/fatal_error_handlers/handlers.h>

#include <ydb-cpp-sdk/library/yson/node/node_io.h>

#include <ydb-cpp-sdk/library/yson/writer.h>
========
#include <src/client/ydb_result/result.h>
#include <src/client/ydb_value/value.h>
#include <src/client/ydb_types/fatal_error_handlers/handlers.h>

#include <src/library/yson/node/node_io.h>

#include <src/library/yson/writer.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/yson_value/ydb_yson_value.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/yson_value/ydb_yson_value.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {

void FormatValueYson(const TValue& value, NYson::TYsonWriter& writer);

std::string FormatValueYson(const TValue& value, NYson::EYsonFormat ysonFormat = NYson::EYsonFormat::Text);

void FormatResultSetYson(const TResultSet& result, NYson::TYsonWriter& writer);

std::string FormatResultSetYson(const TResultSet& result, NYson::EYsonFormat ysonFormat = NYson::EYsonFormat::Text);

} // namespace NYdb
