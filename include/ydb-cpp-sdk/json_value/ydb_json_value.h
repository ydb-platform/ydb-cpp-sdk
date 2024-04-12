#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/json_value/ydb_json_value.h
#include <ydb-cpp-sdk/client/result/result.h>
#include <ydb-cpp-sdk/client/value/value.h>
#include <ydb-cpp-sdk/client/types/fatal_error_handlers/handlers.h>

#include <ydb-cpp-sdk/library/json/writer/json.h>
========
#include <src/client/ydb_result/result.h>
#include <src/client/ydb_value/value.h>
#include <src/client/ydb_types/fatal_error_handlers/handlers.h>

#include <src/library/json/writer/json.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/json_value/ydb_json_value.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/json_value/ydb_json_value.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {

enum class EBinaryStringEncoding {
    /* For binary strings, encode every byte that is not a printable ascii symbol (codes 32-126) as utf-8.
     * Exceptions: \\ \" \b \t \f \r \n
     * Example: "Hello\x01" -> "Hello\\u0001"
    */
    Unicode = 1,

    /* Encode binary strings to base64
    */
    Base64
};

// ====== YDB to json ======
void FormatValueJson(const TValue& value, NJsonWriter::TBuf& writer, EBinaryStringEncoding encoding);

std::string FormatValueJson(const TValue& value, EBinaryStringEncoding encoding);

void FormatResultRowJson(TResultSetParser& parser, const std::vector<TColumn>& columns, NJsonWriter::TBuf& writer,
    EBinaryStringEncoding encoding);

std::string FormatResultRowJson(TResultSetParser& parser, const std::vector<TColumn>& columns,
    EBinaryStringEncoding encoding);

void FormatResultSetJson(const TResultSet& result, IOutputStream* out, EBinaryStringEncoding encoding);

std::string FormatResultSetJson(const TResultSet& result, EBinaryStringEncoding encoding);

// ====== json to YDB ======
TValue JsonToYdbValue(const std::string& jsonString, const TType& type, EBinaryStringEncoding encoding);
TValue JsonToYdbValue(const NJson::TJsonValue& jsonValue, const TType& type, EBinaryStringEncoding encoding);

} // namespace NYdb
