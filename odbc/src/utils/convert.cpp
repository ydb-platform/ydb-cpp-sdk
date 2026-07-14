#include "convert.h"

#include <util/datetime/base.h>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <string>

namespace NYdb::NOdbc {
namespace {

thread_local const char* LastConvertSqlState = nullptr;

void SetNumericOutOfRange() {
    LastConvertSqlState = "22003";
}

bool FitsInt16(int64_t value) {
    return value >= INT16_MIN && value <= INT16_MAX;
}

bool FitsInt32(int64_t value) {
    return value >= INT32_MIN && value <= INT32_MAX;
}

std::optional<int64_t> GetAsInt64(TValueParser& parser, EPrimitiveType type) {
    switch (type) {
        case EPrimitiveType::Bool: return parser.GetBool() ? 1 : 0;
        case EPrimitiveType::Int8: return parser.GetInt8();
        case EPrimitiveType::Uint8: return parser.GetUint8();
        case EPrimitiveType::Int16: return parser.GetInt16();
        case EPrimitiveType::Uint16: return parser.GetUint16();
        case EPrimitiveType::Int32: return parser.GetInt32();
        case EPrimitiveType::Uint32: return parser.GetUint32();
        case EPrimitiveType::Int64: return parser.GetInt64();
        case EPrimitiveType::Uint64: {
            const uint64_t value = parser.GetUint64();
            if (value <= static_cast<uint64_t>(INT64_MAX)) return static_cast<int64_t>(value);
            SetNumericOutOfRange();
            return std::nullopt;
        }
        default: return std::nullopt;
    }
}

std::optional<int64_t> ReadInteger(const TBoundParam& param) {
    if (!param.ParameterValuePtr) return std::nullopt;
    const auto type = param.ValueType;
    if (type == SQL_C_SBIGINT) return *static_cast<const SQLBIGINT*>(param.ParameterValuePtr);
    if (type == SQL_C_UBIGINT) {
        const SQLUBIGINT value = *static_cast<const SQLUBIGINT*>(param.ParameterValuePtr);
        if (value <= static_cast<SQLUBIGINT>(INT64_MAX)) return static_cast<int64_t>(value);
        SetNumericOutOfRange();
        return std::nullopt;
    }
    if (type == SQL_C_LONG || type == SQL_C_SLONG)
        return *static_cast<const SQLINTEGER*>(param.ParameterValuePtr);
    if (type == SQL_C_ULONG)
        return *static_cast<const SQLUINTEGER*>(param.ParameterValuePtr);
    if (type == SQL_C_SHORT || type == SQL_C_SSHORT)
        return *static_cast<const SQLSMALLINT*>(param.ParameterValuePtr);
    if (type == SQL_C_USHORT)
        return *static_cast<const SQLUSMALLINT*>(param.ParameterValuePtr);
    if (type == SQL_C_TINYINT || type == SQL_C_STINYINT)
        return *static_cast<const SQLSCHAR*>(param.ParameterValuePtr);
    if (type == SQL_C_UTINYINT || type == SQL_C_BIT)
        return *static_cast<const SQLCHAR*>(param.ParameterValuePtr);
    return std::nullopt;
}

std::optional<std::string> ReadBytes(const TBoundParam& param) {
    if (!param.ParameterValuePtr) return std::nullopt;
    const char* data = static_cast<const char*>(param.ParameterValuePtr);
    SQLLEN length = param.BufferLength;
    if (param.StrLenOrIndPtr) {
        length = *param.StrLenOrIndPtr;
        if (length == SQL_NTS) return std::string(data);
        if (length < 0) length = param.BufferLength;
    }
    if (length < 0) return std::nullopt;
    return std::string(data, static_cast<size_t>(length));
}

std::optional<EPrimitiveType> ParameterPrimitive(SQLSMALLINT sqlType) {
    switch (sqlType) {
        case SQL_BIGINT: return EPrimitiveType::Int64;
        case SQL_INTEGER: return EPrimitiveType::Int32;
        case SQL_SMALLINT: return EPrimitiveType::Int16;
        case SQL_TINYINT: return EPrimitiveType::Int8;
        case SQL_BIT: return EPrimitiveType::Bool;
        case SQL_REAL: return EPrimitiveType::Float;
        case SQL_FLOAT:
        case SQL_DOUBLE: return EPrimitiveType::Double;
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR: return EPrimitiveType::Utf8;
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY: return EPrimitiveType::String;
        default: return std::nullopt;
    }
}

bool IsNull(const TBoundParam& param) {
    return param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA;
}

} // namespace

SQLRETURN ConvertParam(const TBoundParam& param, TParamValueBuilder& builder) {
    const auto primitive = ParameterPrimitive(param.ParameterType);
    if (!primitive) return SQL_ERROR;
    if (IsNull(param)) {
        builder.EmptyOptional(TTypeBuilder().Primitive(*primitive).Build()).Build();
        return SQL_SUCCESS;
    }

    if (param.ParameterType == SQL_BIGINT || param.ParameterType == SQL_INTEGER
        || param.ParameterType == SQL_SMALLINT || param.ParameterType == SQL_TINYINT
        || param.ParameterType == SQL_BIT) {
        const auto value = ReadInteger(param);
        if (!value) return SQL_ERROR;
        switch (param.ParameterType) {
            case SQL_BIGINT: builder.OptionalInt64(*value); break;
            case SQL_INTEGER:
                if (!FitsInt32(*value)) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalInt32(static_cast<int32_t>(*value)); break;
            case SQL_SMALLINT:
                if (!FitsInt16(*value)) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalInt16(static_cast<int16_t>(*value)); break;
            case SQL_TINYINT:
                if (*value < INT8_MIN || *value > INT8_MAX) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalInt8(static_cast<int8_t>(*value)); break;
            case SQL_BIT:
                if (*value != 0 && *value != 1) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalBool(*value != 0); break;
        }
    } else if (param.ParameterType == SQL_REAL) {
        if (param.ValueType != SQL_C_FLOAT || !param.ParameterValuePtr) return SQL_ERROR;
        builder.OptionalFloat(*static_cast<const SQLREAL*>(param.ParameterValuePtr));
    } else if (param.ParameterType == SQL_FLOAT || param.ParameterType == SQL_DOUBLE) {
        if (param.ValueType != SQL_C_DOUBLE || !param.ParameterValuePtr) return SQL_ERROR;
        builder.OptionalDouble(*static_cast<const SQLDOUBLE*>(param.ParameterValuePtr));
    } else {
        const auto bytes = ReadBytes(param);
        if (!bytes) return SQL_ERROR;
        if (param.ValueType == SQL_C_CHAR
            && (param.ParameterType == SQL_CHAR || param.ParameterType == SQL_VARCHAR
                || param.ParameterType == SQL_LONGVARCHAR)) {
            builder.OptionalUtf8(*bytes);
        } else if (param.ValueType == SQL_C_BINARY
                   && (param.ParameterType == SQL_BINARY || param.ParameterType == SQL_VARBINARY
                       || param.ParameterType == SQL_LONGVARBINARY)) {
            builder.OptionalString(*bytes);
        } else {
            return SQL_ERROR;
        }
    }
    builder.Build();
    return SQL_SUCCESS;
}

SQLRETURN ConvertColumn(TValueParser& parser, SQLSMALLINT targetType, SQLPOINTER targetValue,
                        SQLLEN bufferLength, SQLLEN* strLenOrInd, SQLLEN* offset) {
    LastConvertSqlState = nullptr;
    if (bufferLength < 0) {
        LastConvertSqlState = "HY090";
        return SQL_ERROR;
    }
    if (parser.IsNull()) {
        if (!strLenOrInd) {
            LastConvertSqlState = "22002";
            return SQL_ERROR;
        }
        *strLenOrInd = SQL_NULL_DATA;
        return SQL_SUCCESS;
    }
    if (parser.GetKind() == TTypeParser::ETypeKind::Optional) {
        parser.OpenOptional();
        const SQLRETURN result = ConvertColumn(
            parser, targetType, targetValue, bufferLength, strLenOrInd, offset);
        parser.CloseOptional();
        return result;
    }
    if (parser.GetKind() != TTypeParser::ETypeKind::Primitive) return SQL_ERROR;
    const EPrimitiveType ydbType = parser.GetPrimitiveType();

    if (targetType == SQL_C_SHORT || targetType == SQL_C_SSHORT
        || targetType == SQL_C_LONG || targetType == SQL_C_SLONG
        || targetType == SQL_C_SBIGINT || targetType == SQL_C_BIT) {
        const auto raw = GetAsInt64(parser, ydbType);
        if (!raw) return SQL_ERROR;
        if (targetType == SQL_C_SHORT || targetType == SQL_C_SSHORT) {
            if (!FitsInt16(*raw)) { SetNumericOutOfRange(); return SQL_ERROR; }
            if (targetValue) *static_cast<SQLSMALLINT*>(targetValue) = static_cast<SQLSMALLINT>(*raw);
            if (strLenOrInd) *strLenOrInd = sizeof(SQLSMALLINT);
        } else if (targetType == SQL_C_LONG || targetType == SQL_C_SLONG) {
            if (!FitsInt32(*raw)) { SetNumericOutOfRange(); return SQL_ERROR; }
            if (targetValue) *static_cast<SQLINTEGER*>(targetValue) = static_cast<SQLINTEGER>(*raw);
            if (strLenOrInd) *strLenOrInd = sizeof(SQLINTEGER);
        } else if (targetType == SQL_C_SBIGINT) {
            if (targetValue) *static_cast<SQLBIGINT*>(targetValue) = *raw;
            if (strLenOrInd) *strLenOrInd = sizeof(SQLBIGINT);
        } else {
            if (*raw != 0 && *raw != 1) { SetNumericOutOfRange(); return SQL_ERROR; }
            if (targetValue) *static_cast<SQLCHAR*>(targetValue) = *raw != 0;
            if (strLenOrInd) *strLenOrInd = sizeof(SQLCHAR);
        }
        return SQL_SUCCESS;
    }
    if (targetType == SQL_C_DOUBLE) {
        double value;
        if (ydbType == EPrimitiveType::Double) value = parser.GetDouble();
        else if (ydbType == EPrimitiveType::Float) value = parser.GetFloat();
        else return SQL_ERROR;
        if (targetValue) *static_cast<SQLDOUBLE*>(targetValue) = value;
        if (strLenOrInd) *strLenOrInd = sizeof(SQLDOUBLE);
        return SQL_SUCCESS;
    }
    if (targetType != SQL_C_CHAR) return SQL_ERROR;

    std::string text;
    switch (ydbType) {
        case EPrimitiveType::Utf8: text = parser.GetUtf8(); break;
        case EPrimitiveType::String: text = parser.GetString(); break;
        case EPrimitiveType::Json: text = parser.GetJson(); break;
        case EPrimitiveType::JsonDocument: text = parser.GetJsonDocument(); break;
        case EPrimitiveType::Bool: text = parser.GetBool() ? "1" : "0"; break;
        case EPrimitiveType::Int8: text = std::to_string(parser.GetInt8()); break;
        case EPrimitiveType::Uint8: text = std::to_string(parser.GetUint8()); break;
        case EPrimitiveType::Int16: text = std::to_string(parser.GetInt16()); break;
        case EPrimitiveType::Uint16: text = std::to_string(parser.GetUint16()); break;
        case EPrimitiveType::Int32: text = std::to_string(parser.GetInt32()); break;
        case EPrimitiveType::Uint32: text = std::to_string(parser.GetUint32()); break;
        case EPrimitiveType::Int64: text = std::to_string(parser.GetInt64()); break;
        case EPrimitiveType::Uint64: text = std::to_string(parser.GetUint64()); break;
        case EPrimitiveType::Float: text = std::to_string(parser.GetFloat()); break;
        case EPrimitiveType::Double: text = std::to_string(parser.GetDouble()); break;
        case EPrimitiveType::Date: {
            const TString value = parser.GetDate().FormatGmTime("%Y-%m-%d");
            text.assign(value.data(), value.size()); break;
        }
        case EPrimitiveType::Date32: {
            const auto days = parser.GetDate32().time_since_epoch().count();
            if (days < 0) return SQL_ERROR;
            const TString value = TInstant::Days(static_cast<ui64>(days)).FormatGmTime("%Y-%m-%d");
            text.assign(value.data(), value.size()); break;
        }
        case EPrimitiveType::Datetime: {
            const TString value = parser.GetDatetime().FormatGmTime("%Y-%m-%d %H:%M:%S");
            text.assign(value.data(), value.size()); break;
        }
        case EPrimitiveType::Datetime64: {
            const auto seconds = parser.GetDatetime64().time_since_epoch().count();
            if (seconds < 0) return SQL_ERROR;
            const TString value = TInstant::Seconds(static_cast<ui64>(seconds))
                .FormatGmTime("%Y-%m-%d %H:%M:%S");
            text.assign(value.data(), value.size()); break;
        }
        case EPrimitiveType::Timestamp: {
            const TString value = parser.GetTimestamp().FormatGmTime("%Y-%m-%d %H:%M:%S");
            text.assign(value.data(), value.size()); break;
        }
        case EPrimitiveType::Timestamp64: {
            const auto micros = parser.GetTimestamp64().time_since_epoch().count();
            if (micros < 0) return SQL_ERROR;
            const TString value = TInstant::MicroSeconds(static_cast<ui64>(micros))
                .FormatGmTime("%Y-%m-%d %H:%M:%S");
            text.assign(value.data(), value.size()); break;
        }
        case EPrimitiveType::TzDate: text = parser.GetTzDate(); break;
        case EPrimitiveType::TzDatetime: text = parser.GetTzDatetime(); break;
        case EPrimitiveType::TzTimestamp: text = parser.GetTzTimestamp(); break;
        default: return SQL_ERROR;
    }

    if (offset && *offset < 0) return SQL_NO_DATA;
    const SQLLEN start = offset ? *offset : 0;
    const SQLLEN remaining = static_cast<SQLLEN>(text.size()) - start;
    if (targetValue && bufferLength > 0) {
        const SQLLEN copied = std::min(remaining, bufferLength - 1);
        std::memcpy(targetValue, text.data() + start, static_cast<size_t>(copied));
        static_cast<char*>(targetValue)[copied] = 0;
        if (offset) *offset = copied == remaining ? -1 : start + copied;
    }
    if (strLenOrInd) *strLenOrInd = remaining;
    return targetValue && bufferLength > 0 && remaining >= bufferLength
        ? SQL_SUCCESS_WITH_INFO
        : SQL_SUCCESS;
}

const char* ConsumeLastConvertSqlState() {
    const char* result = LastConvertSqlState;
    LastConvertSqlState = nullptr;
    return result;
}

} // namespace NYdb::NOdbc
