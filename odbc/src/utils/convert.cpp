#include "convert.h"
#include "sql_type_map.h"
#include "util.h"

#include <util/charset/wide.h>
#include <util/datetime/base.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace NYdb::NOdbc {
namespace {

thread_local const char* LastConvertSqlState = nullptr;

void SetNumericOutOfRange() {
    LastConvertSqlState = "22003";
}

void SetInvalidCharacterValue() {
    LastConvertSqlState = "22018";
}

void SetInvalidDatetime() {
    LastConvertSqlState = "22007";
}

bool FitsInt16(int64_t value) {
    return value >= INT16_MIN && value <= INT16_MAX;
}

bool FitsInt32(int64_t value) {
    return value >= INT32_MIN && value <= INT32_MAX;
}

template <typename T>
bool FitsUnsigned(uint64_t value) {
    return value <= static_cast<uint64_t>(std::numeric_limits<T>::max());
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

std::optional<uint64_t> GetAsUint64(TValueParser& parser, EPrimitiveType type) {
    switch (type) {
        case EPrimitiveType::Bool: return parser.GetBool() ? 1 : 0;
        case EPrimitiveType::Uint8: return parser.GetUint8();
        case EPrimitiveType::Uint16: return parser.GetUint16();
        case EPrimitiveType::Uint32: return parser.GetUint32();
        case EPrimitiveType::Uint64: return parser.GetUint64();
        case EPrimitiveType::Int8: {
            const auto value = parser.GetInt8();
            if (value >= 0) return static_cast<uint64_t>(value);
            break;
        }
        case EPrimitiveType::Int16: {
            const auto value = parser.GetInt16();
            if (value >= 0) return static_cast<uint64_t>(value);
            break;
        }
        case EPrimitiveType::Int32: {
            const auto value = parser.GetInt32();
            if (value >= 0) return static_cast<uint64_t>(value);
            break;
        }
        case EPrimitiveType::Int64: {
            const auto value = parser.GetInt64();
            if (value >= 0) return static_cast<uint64_t>(value);
            break;
        }
        default: return std::nullopt;
    }
    SetNumericOutOfRange();
    return std::nullopt;
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

std::optional<uint64_t> ReadUnsignedInteger(const TBoundParam& param) {
    if (!param.ParameterValuePtr) return std::nullopt;
    const auto type = param.ValueType;
    if (type == SQL_C_UBIGINT)
        return *static_cast<const SQLUBIGINT*>(param.ParameterValuePtr);
    if (type == SQL_C_ULONG)
        return *static_cast<const SQLUINTEGER*>(param.ParameterValuePtr);
    if (type == SQL_C_USHORT)
        return *static_cast<const SQLUSMALLINT*>(param.ParameterValuePtr);
    if (type == SQL_C_UTINYINT || type == SQL_C_BIT)
        return *static_cast<const SQLCHAR*>(param.ParameterValuePtr);
    const auto value = ReadInteger(param);
    if (value && *value >= 0) return static_cast<uint64_t>(*value);
    if (value) SetNumericOutOfRange();
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

std::optional<std::string> ReadText(const TBoundParam& param) {
    if (param.ValueType == SQL_C_CHAR) {
        return ReadBytes(param);
    }
    if (param.ValueType != SQL_C_WCHAR || !param.ParameterValuePtr) {
        return std::nullopt;
    }
    SQLLEN length = param.StrLenOrIndPtr ? *param.StrLenOrIndPtr : param.BufferLength;
    if (length == SQL_NTS) {
        return GetString(static_cast<SQLWCHAR*>(param.ParameterValuePtr), SQL_NTS);
    }
    if (length < 0 || length % static_cast<SQLLEN>(sizeof(SQLWCHAR)) != 0) {
        SetInvalidCharacterValue();
        return std::nullopt;
    }
    return GetString(
        static_cast<SQLWCHAR*>(param.ParameterValuePtr),
        static_cast<SQLINTEGER>(length / sizeof(SQLWCHAR)));
}

std::optional<std::string> ReadNumericStruct(const SQL_NUMERIC_STRUCT& numeric) {
    unsigned __int128 magnitude = 0;
    for (size_t i = 0; i < SQL_MAX_NUMERIC_LEN; ++i) {
        magnitude |= static_cast<unsigned __int128>(numeric.val[i]) << (i * 8);
    }
    std::string digits;
    do {
        digits.push_back(static_cast<char>('0' + magnitude % 10));
        magnitude /= 10;
    } while (magnitude != 0);
    std::reverse(digits.begin(), digits.end());
    const int scale = numeric.scale;
    if (scale < 0) {
        digits.append(static_cast<size_t>(-scale), '0');
    } else if (scale > 0) {
        if (digits.size() <= static_cast<size_t>(scale)) {
            digits.insert(0, static_cast<size_t>(scale) + 1 - digits.size(), '0');
        }
        digits.insert(digits.size() - static_cast<size_t>(scale), 1, '.');
    }
    if (numeric.sign == 0 && digits != "0") {
        digits.insert(digits.begin(), '-');
    }
    return digits;
}

std::optional<std::string> ReadDecimalText(const TBoundParam& param) {
    if (auto text = ReadText(param)) return text;
    if (param.ValueType == SQL_C_NUMERIC && param.ParameterValuePtr) {
        return ReadNumericStruct(*static_cast<const SQL_NUMERIC_STRUCT*>(param.ParameterValuePtr));
    }
    if (const auto value = ReadInteger(param)) return std::to_string(*value);
    std::ostringstream out;
    if (param.ValueType == SQL_C_FLOAT && param.ParameterValuePtr) {
        out << std::setprecision(std::numeric_limits<float>::max_digits10)
            << *static_cast<const SQLREAL*>(param.ParameterValuePtr);
        return out.str();
    }
    if (param.ValueType == SQL_C_DOUBLE && param.ParameterValuePtr) {
        out << std::setprecision(std::numeric_limits<double>::max_digits10)
            << *static_cast<const SQLDOUBLE*>(param.ParameterValuePtr);
        return out.str();
    }
    return std::nullopt;
}

std::string FormatDecimalText(const TDecimalValue& decimal) {
    std::string text = decimal.ToString();
    const size_t point = text.find('.');
    const size_t fractionSize = point == std::string::npos ? 0 : text.size() - point - 1;
    if (decimal.DecimalType_.Scale > fractionSize) {
        if (point == std::string::npos) {
            text.push_back('.');
        }
        text.append(decimal.DecimalType_.Scale - fractionSize, '0');
    }
    return text;
}

std::optional<TInstant> ReadTemporal(const TBoundParam& param, EParamYdbType type) {
    if (!param.ParameterValuePtr) return std::nullopt;
    char text[64] = {};
    if (type == EParamYdbType::Date
        && (param.ValueType == SQL_C_TYPE_DATE || param.ValueType == SQL_C_DATE)) {
        const auto& value = *static_cast<const SQL_DATE_STRUCT*>(param.ParameterValuePtr);
        std::snprintf(text, sizeof(text), "%04d-%02u-%02uT00:00:00Z",
                      value.year, value.month, value.day);
    } else if (type == EParamYdbType::Datetime
               && (param.ValueType == SQL_C_TYPE_TIME || param.ValueType == SQL_C_TIME)) {
        const auto& value = *static_cast<const SQL_TIME_STRUCT*>(param.ParameterValuePtr);
        std::snprintf(text, sizeof(text), "1970-01-01T%02u:%02u:%02uZ",
                      value.hour, value.minute, value.second);
    } else if (type == EParamYdbType::Timestamp
               && (param.ValueType == SQL_C_TYPE_TIMESTAMP || param.ValueType == SQL_C_TIMESTAMP)) {
        const auto& value = *static_cast<const SQL_TIMESTAMP_STRUCT*>(param.ParameterValuePtr);
        std::snprintf(text, sizeof(text), "%04d-%02u-%02uT%02u:%02u:%02u.%09uZ",
                      value.year, value.month, value.day, value.hour, value.minute,
                      value.second, value.fraction);
    } else {
        SetInvalidDatetime();
        return std::nullopt;
    }
    TInstant instant;
    if (!TInstant::TryParseIso8601(text, instant)) {
        SetInvalidDatetime();
        return std::nullopt;
    }
    return instant;
}

SQLRETURN CopyVariable(
    const void* data,
    SQLLEN size,
    SQLLEN terminatorSize,
    SQLLEN unitSize,
    SQLPOINTER targetValue,
    SQLLEN bufferLength,
    SQLLEN* strLenOrInd,
    SQLLEN* offset) {
    if (offset && *offset < 0) return SQL_NO_DATA;
    const SQLLEN start = offset ? *offset : 0;
    if (start < 0 || start > size || start % unitSize != 0) {
        LastConvertSqlState = "HY090";
        return SQL_ERROR;
    }
    const SQLLEN remaining = size - start;
    SQLLEN copied = 0;
    if (targetValue && bufferLength >= terminatorSize) {
        SQLLEN capacity = bufferLength - terminatorSize;
        capacity -= capacity % unitSize;
        copied = std::min(remaining, capacity);
        if (copied > 0) {
            std::memcpy(targetValue, static_cast<const char*>(data) + start,
                        static_cast<size_t>(copied));
        }
        if (terminatorSize > 0) {
            std::memset(static_cast<char*>(targetValue) + copied, 0,
                        static_cast<size_t>(terminatorSize));
        }
    }
    if (offset) *offset = copied == remaining ? -1 : start + copied;
    if (strLenOrInd) *strLenOrInd = remaining;
    return targetValue && copied < remaining ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;
}

std::optional<TInstant> GetAsInstant(TValueParser& parser, EPrimitiveType type) {
    switch (type) {
        case EPrimitiveType::Date: return parser.GetDate();
        case EPrimitiveType::Datetime: return parser.GetDatetime();
        case EPrimitiveType::Timestamp: return parser.GetTimestamp();
        case EPrimitiveType::Date32: {
            const auto days = parser.GetDate32().time_since_epoch().count();
            if (days >= 0) return TInstant::Days(static_cast<uint64_t>(days));
            break;
        }
        case EPrimitiveType::Datetime64: {
            const auto seconds = parser.GetDatetime64().time_since_epoch().count();
            if (seconds >= 0) return TInstant::Seconds(static_cast<uint64_t>(seconds));
            break;
        }
        case EPrimitiveType::Timestamp64: {
            const auto micros = parser.GetTimestamp64().time_since_epoch().count();
            if (micros >= 0) return TInstant::MicroSeconds(static_cast<uint64_t>(micros));
            break;
        }
        case EPrimitiveType::TzDate:
        case EPrimitiveType::TzDatetime:
        case EPrimitiveType::TzTimestamp: {
            const std::string value = type == EPrimitiveType::TzDate
                ? parser.GetTzDate()
                : type == EPrimitiveType::TzDatetime
                    ? parser.GetTzDatetime()
                    : parser.GetTzTimestamp();
            TInstant instant;
            if (TInstant::TryParseIso8601(value, instant)) return instant;
            break;
        }
        default: return std::nullopt;
    }
    SetInvalidDatetime();
    return std::nullopt;
}

SQLRETURN WriteTemporal(
    TValueParser& parser,
    EPrimitiveType type,
    SQLSMALLINT targetType,
    SQLPOINTER targetValue,
    SQLLEN* strLenOrInd) {
    const auto instant = GetAsInstant(parser, type);
    if (!instant) return SQL_ERROR;
    struct tm value = {};
    if (!instant->GmTime(&value)) {
        SetInvalidDatetime();
        return SQL_ERROR;
    }
    if (targetType == SQL_C_TYPE_DATE || targetType == SQL_C_DATE) {
        if (targetValue) {
            auto& out = *static_cast<SQL_DATE_STRUCT*>(targetValue);
            out.year = static_cast<SQLSMALLINT>(value.tm_year + 1900);
            out.month = static_cast<SQLUSMALLINT>(value.tm_mon + 1);
            out.day = static_cast<SQLUSMALLINT>(value.tm_mday);
        }
        if (strLenOrInd) *strLenOrInd = sizeof(SQL_DATE_STRUCT);
    } else if (targetType == SQL_C_TYPE_TIME || targetType == SQL_C_TIME) {
        if (targetValue) {
            auto& out = *static_cast<SQL_TIME_STRUCT*>(targetValue);
            out.hour = static_cast<SQLUSMALLINT>(value.tm_hour);
            out.minute = static_cast<SQLUSMALLINT>(value.tm_min);
            out.second = static_cast<SQLUSMALLINT>(value.tm_sec);
        }
        if (strLenOrInd) *strLenOrInd = sizeof(SQL_TIME_STRUCT);
    } else {
        if (targetValue) {
            auto& out = *static_cast<SQL_TIMESTAMP_STRUCT*>(targetValue);
            out.year = static_cast<SQLSMALLINT>(value.tm_year + 1900);
            out.month = static_cast<SQLUSMALLINT>(value.tm_mon + 1);
            out.day = static_cast<SQLUSMALLINT>(value.tm_mday);
            out.hour = static_cast<SQLUSMALLINT>(value.tm_hour);
            out.minute = static_cast<SQLUSMALLINT>(value.tm_min);
            out.second = static_cast<SQLUSMALLINT>(value.tm_sec);
            out.fraction = instant->MicroSecondsOfSecond() * 1000;
        }
        if (strLenOrInd) *strLenOrInd = sizeof(SQL_TIMESTAMP_STRUCT);
    }
    return SQL_SUCCESS;
}

SQLRETURN WriteNumericStruct(
    std::string text,
    SQLPOINTER targetValue,
    SQLLEN* strLenOrInd,
    SQLULEN precision,
    SQLSMALLINT scale) {
    bool positive = true;
    if (!text.empty() && (text.front() == '-' || text.front() == '+')) {
        positive = text.front() != '-';
        text.erase(text.begin());
    }
    const auto point = text.find('.');
    if (point != std::string::npos) text.erase(point, 1);
    while (!text.empty() && text.front() == '0') text.erase(text.begin());
    if (text.empty()) text = "0";
    unsigned __int128 magnitude = 0;
    for (const char ch : text) {
        if (ch < '0' || ch > '9') {
            SetInvalidCharacterValue();
            return SQL_ERROR;
        }
        const auto previous = magnitude;
        magnitude = magnitude * 10 + static_cast<unsigned>(ch - '0');
        if (magnitude < previous) {
            SetNumericOutOfRange();
            return SQL_ERROR;
        }
    }
    if (targetValue) {
        auto& out = *static_cast<SQL_NUMERIC_STRUCT*>(targetValue);
        std::memset(&out, 0, sizeof(out));
        out.precision = static_cast<SQLCHAR>(precision);
        out.scale = static_cast<SQLSCHAR>(scale);
        out.sign = positive ? 1 : 0;
        for (size_t i = 0; i < SQL_MAX_NUMERIC_LEN; ++i) {
            out.val[i] = static_cast<SQLCHAR>(magnitude & 0xff);
            magnitude >>= 8;
        }
        if (magnitude != 0) {
            SetNumericOutOfRange();
            return SQL_ERROR;
        }
    }
    if (strLenOrInd) *strLenOrInd = sizeof(SQL_NUMERIC_STRUCT);
    return SQL_SUCCESS;
}

bool IsSignedIntegerTarget(SQLSMALLINT type) {
    return type == SQL_C_TINYINT || type == SQL_C_STINYINT
        || type == SQL_C_SHORT || type == SQL_C_SSHORT
        || type == SQL_C_LONG || type == SQL_C_SLONG
        || type == SQL_C_SBIGINT || type == SQL_C_BIT;
}

bool IsUnsignedIntegerTarget(SQLSMALLINT type) {
    return type == SQL_C_UTINYINT || type == SQL_C_USHORT
        || type == SQL_C_ULONG || type == SQL_C_UBIGINT;
}

SQLRETURN WriteSignedInteger(
    int64_t value, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN* strLenOrInd) {
    if (targetType == SQL_C_TINYINT || targetType == SQL_C_STINYINT) {
        if (value < INT8_MIN || value > INT8_MAX) { SetNumericOutOfRange(); return SQL_ERROR; }
        if (targetValue) *static_cast<SQLSCHAR*>(targetValue) = static_cast<SQLSCHAR>(value);
        if (strLenOrInd) *strLenOrInd = sizeof(SQLSCHAR);
    } else if (targetType == SQL_C_SHORT || targetType == SQL_C_SSHORT) {
        if (!FitsInt16(value)) { SetNumericOutOfRange(); return SQL_ERROR; }
        if (targetValue) *static_cast<SQLSMALLINT*>(targetValue) = static_cast<SQLSMALLINT>(value);
        if (strLenOrInd) *strLenOrInd = sizeof(SQLSMALLINT);
    } else if (targetType == SQL_C_LONG || targetType == SQL_C_SLONG) {
        if (!FitsInt32(value)) { SetNumericOutOfRange(); return SQL_ERROR; }
        if (targetValue) *static_cast<SQLINTEGER*>(targetValue) = static_cast<SQLINTEGER>(value);
        if (strLenOrInd) *strLenOrInd = sizeof(SQLINTEGER);
    } else if (targetType == SQL_C_SBIGINT) {
        if (targetValue) *static_cast<SQLBIGINT*>(targetValue) = value;
        if (strLenOrInd) *strLenOrInd = sizeof(SQLBIGINT);
    } else if (targetType == SQL_C_BIT) {
        if (value != 0 && value != 1) { SetNumericOutOfRange(); return SQL_ERROR; }
        if (targetValue) *static_cast<SQLCHAR*>(targetValue) = value != 0;
        if (strLenOrInd) *strLenOrInd = sizeof(SQLCHAR);
    } else {
        return SQL_ERROR;
    }
    return SQL_SUCCESS;
}

SQLRETURN WriteUnsignedInteger(
    uint64_t value, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN* strLenOrInd) {
    if (targetType == SQL_C_UTINYINT) {
        if (!FitsUnsigned<SQLCHAR>(value)) { SetNumericOutOfRange(); return SQL_ERROR; }
        if (targetValue) *static_cast<SQLCHAR*>(targetValue) = static_cast<SQLCHAR>(value);
        if (strLenOrInd) *strLenOrInd = sizeof(SQLCHAR);
    } else if (targetType == SQL_C_USHORT) {
        if (!FitsUnsigned<SQLUSMALLINT>(value)) { SetNumericOutOfRange(); return SQL_ERROR; }
        if (targetValue) *static_cast<SQLUSMALLINT*>(targetValue) = static_cast<SQLUSMALLINT>(value);
        if (strLenOrInd) *strLenOrInd = sizeof(SQLUSMALLINT);
    } else if (targetType == SQL_C_ULONG) {
        if (!FitsUnsigned<SQLUINTEGER>(value)) { SetNumericOutOfRange(); return SQL_ERROR; }
        if (targetValue) *static_cast<SQLUINTEGER*>(targetValue) = static_cast<SQLUINTEGER>(value);
        if (strLenOrInd) *strLenOrInd = sizeof(SQLUINTEGER);
    } else if (targetType == SQL_C_UBIGINT) {
        if (targetValue) *static_cast<SQLUBIGINT*>(targetValue) = value;
        if (strLenOrInd) *strLenOrInd = sizeof(SQLUBIGINT);
    } else {
        return SQL_ERROR;
    }
    return SQL_SUCCESS;
}

std::string DecimalIntegralPart(const std::string& text) {
    const size_t point = text.find('.');
    return point == std::string::npos ? text : text.substr(0, point);
}

std::optional<EPrimitiveType> ParamPrimitive(EParamYdbType type) {
    switch (type) {
        case EParamYdbType::Bool: return EPrimitiveType::Bool;
        case EParamYdbType::Int8: return EPrimitiveType::Int8;
        case EParamYdbType::Uint8: return EPrimitiveType::Uint8;
        case EParamYdbType::Int16: return EPrimitiveType::Int16;
        case EParamYdbType::Uint16: return EPrimitiveType::Uint16;
        case EParamYdbType::Int32: return EPrimitiveType::Int32;
        case EParamYdbType::Uint32: return EPrimitiveType::Uint32;
        case EParamYdbType::Int64: return EPrimitiveType::Int64;
        case EParamYdbType::Uint64: return EPrimitiveType::Uint64;
        case EParamYdbType::Float: return EPrimitiveType::Float;
        case EParamYdbType::Double: return EPrimitiveType::Double;
        case EParamYdbType::Utf8: return EPrimitiveType::Utf8;
        case EParamYdbType::String: return EPrimitiveType::String;
        case EParamYdbType::Date: return EPrimitiveType::Date;
        case EParamYdbType::Datetime: return EPrimitiveType::Datetime;
        case EParamYdbType::Timestamp: return EPrimitiveType::Timestamp;
        case EParamYdbType::Decimal: return std::nullopt;
    }
    return std::nullopt;
}

bool IsNull(const TBoundParam& param) {
    return param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA;
}

} // namespace

SQLRETURN ConvertParam(const TBoundParam& param, TParamValueBuilder& builder) {
    LastConvertSqlState = nullptr;
    const auto type = ResolveParamType(param);
    if (!type) return SQL_ERROR;
    if (IsNull(param)) {
        TTypeBuilder itemType;
        if (type->Type == EParamYdbType::Decimal) {
            itemType.Decimal(TDecimalType(
                static_cast<uint8_t>(type->Precision), static_cast<uint8_t>(type->Scale)));
        } else {
            const auto primitive = ParamPrimitive(type->Type);
            if (!primitive) return SQL_ERROR;
            itemType.Primitive(*primitive);
        }
        builder.EmptyOptional(itemType.Build()).Build();
        return SQL_SUCCESS;
    }

    switch (type->Type) {
        case EParamYdbType::Bool: {
            const auto value = ReadInteger(param);
            if (!value || (*value != 0 && *value != 1)) {
                SetNumericOutOfRange();
                return SQL_ERROR;
            }
            builder.OptionalBool(*value != 0);
            break;
        }
        case EParamYdbType::Int8:
        case EParamYdbType::Int16:
        case EParamYdbType::Int32:
        case EParamYdbType::Int64: {
            const auto value = ReadInteger(param);
            if (!value) return SQL_ERROR;
            if (type->Type == EParamYdbType::Int8) {
                if (*value < INT8_MIN || *value > INT8_MAX) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalInt8(static_cast<int8_t>(*value));
            } else if (type->Type == EParamYdbType::Int16) {
                if (!FitsInt16(*value)) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalInt16(static_cast<int16_t>(*value));
            } else if (type->Type == EParamYdbType::Int32) {
                if (!FitsInt32(*value)) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalInt32(static_cast<int32_t>(*value));
            } else {
                builder.OptionalInt64(*value);
            }
            break;
        }
        case EParamYdbType::Uint8:
        case EParamYdbType::Uint16:
        case EParamYdbType::Uint32:
        case EParamYdbType::Uint64: {
            const auto value = ReadUnsignedInteger(param);
            if (!value) return SQL_ERROR;
            if (type->Type == EParamYdbType::Uint8) {
                if (!FitsUnsigned<uint8_t>(*value)) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalUint8(static_cast<uint8_t>(*value));
            } else if (type->Type == EParamYdbType::Uint16) {
                if (!FitsUnsigned<uint16_t>(*value)) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalUint16(static_cast<uint16_t>(*value));
            } else if (type->Type == EParamYdbType::Uint32) {
                if (!FitsUnsigned<uint32_t>(*value)) { SetNumericOutOfRange(); return SQL_ERROR; }
                builder.OptionalUint32(static_cast<uint32_t>(*value));
            } else {
                builder.OptionalUint64(*value);
            }
            break;
        }
        case EParamYdbType::Float: {
            if (!param.ParameterValuePtr) return SQL_ERROR;
            if (param.ValueType == SQL_C_FLOAT) {
                builder.OptionalFloat(*static_cast<const SQLREAL*>(param.ParameterValuePtr));
            } else if (param.ValueType == SQL_C_DOUBLE) {
                builder.OptionalFloat(static_cast<float>(
                    *static_cast<const SQLDOUBLE*>(param.ParameterValuePtr)));
            } else {
                return SQL_ERROR;
            }
            break;
        }
        case EParamYdbType::Double: {
            if (!param.ParameterValuePtr) return SQL_ERROR;
            if (param.ValueType == SQL_C_DOUBLE) {
                builder.OptionalDouble(*static_cast<const SQLDOUBLE*>(param.ParameterValuePtr));
            } else if (param.ValueType == SQL_C_FLOAT) {
                builder.OptionalDouble(*static_cast<const SQLREAL*>(param.ParameterValuePtr));
            } else {
                return SQL_ERROR;
            }
            break;
        }
        case EParamYdbType::Decimal: {
            const auto text = ReadDecimalText(param);
            if (!text) return SQL_ERROR;
            try {
                builder.BeginOptional()
                    .Decimal(TDecimalValue(
                        *text, static_cast<uint8_t>(type->Precision), static_cast<uint8_t>(type->Scale)))
                    .EndOptional();
            } catch (...) {
                SetInvalidCharacterValue();
                return SQL_ERROR;
            }
            break;
        }
        case EParamYdbType::Utf8: {
            const auto text = ReadText(param);
            if (!text) return SQL_ERROR;
            builder.OptionalUtf8(*text);
            break;
        }
        case EParamYdbType::String: {
            if (param.ValueType != SQL_C_BINARY) return SQL_ERROR;
            const auto bytes = ReadBytes(param);
            if (!bytes) return SQL_ERROR;
            builder.OptionalString(*bytes);
            break;
        }
        case EParamYdbType::Date:
        case EParamYdbType::Datetime:
        case EParamYdbType::Timestamp: {
            const auto instant = ReadTemporal(param, type->Type);
            if (!instant) return SQL_ERROR;
            if (type->Type == EParamYdbType::Date) builder.OptionalDate(*instant);
            else if (type->Type == EParamYdbType::Datetime) builder.OptionalDatetime(*instant);
            else builder.OptionalTimestamp(*instant);
            break;
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
    if (parser.GetKind() == TTypeParser::ETypeKind::Decimal) {
        const TDecimalValue decimal = parser.GetDecimal();
        const std::string text = FormatDecimalText(decimal);
        if (targetType == SQL_C_DOUBLE || targetType == SQL_C_FLOAT) {
            try {
                const double value = std::stod(text);
                if (targetType == SQL_C_DOUBLE) {
                    if (targetValue) *static_cast<SQLDOUBLE*>(targetValue) = value;
                    if (strLenOrInd) *strLenOrInd = sizeof(SQLDOUBLE);
                } else {
                    if (targetValue) *static_cast<SQLREAL*>(targetValue) = static_cast<float>(value);
                    if (strLenOrInd) *strLenOrInd = sizeof(SQLREAL);
                }
                return SQL_SUCCESS;
            } catch (...) {
                SetNumericOutOfRange();
                return SQL_ERROR;
            }
        }
        if (targetType == SQL_C_NUMERIC) {
            return WriteNumericStruct(
                text, targetValue, strLenOrInd,
                decimal.DecimalType_.Precision, decimal.DecimalType_.Scale);
        }
        if (IsSignedIntegerTarget(targetType)) {
            try {
                size_t parsed = 0;
                const std::string integral = DecimalIntegralPart(text);
                const int64_t value = std::stoll(integral, &parsed);
                if (parsed != integral.size()) throw std::invalid_argument("decimal");
                return WriteSignedInteger(value, targetType, targetValue, strLenOrInd);
            } catch (...) {
                SetNumericOutOfRange();
                return SQL_ERROR;
            }
        }
        if (IsUnsignedIntegerTarget(targetType)) {
            try {
                size_t parsed = 0;
                const std::string integral = DecimalIntegralPart(text);
                if (!integral.empty() && integral.front() == '-') {
                    throw std::out_of_range("negative decimal");
                }
                const uint64_t value = std::stoull(integral, &parsed);
                if (parsed != integral.size()) throw std::invalid_argument("decimal");
                return WriteUnsignedInteger(value, targetType, targetValue, strLenOrInd);
            } catch (...) {
                SetNumericOutOfRange();
                return SQL_ERROR;
            }
        }
        if (targetType == SQL_C_CHAR) {
            return CopyVariable(text.data(), static_cast<SQLLEN>(text.size()), 1, 1,
                                targetValue, bufferLength, strLenOrInd, offset);
        }
        if (targetType == SQL_C_WCHAR) {
            const TUtf16String wide = UTF8ToWide(text);
            static_assert(sizeof(TUtf16String::value_type) == sizeof(SQLWCHAR));
            return CopyVariable(wide.data(), static_cast<SQLLEN>(wide.size() * sizeof(SQLWCHAR)),
                                sizeof(SQLWCHAR), sizeof(SQLWCHAR), targetValue, bufferLength,
                                strLenOrInd, offset);
        }
        return SQL_ERROR;
    }
    if (parser.GetKind() != TTypeParser::ETypeKind::Primitive) return SQL_ERROR;
    const EPrimitiveType ydbType = parser.GetPrimitiveType();

    if (IsSignedIntegerTarget(targetType)) {
        const auto raw = GetAsInt64(parser, ydbType);
        if (!raw) return SQL_ERROR;
        return WriteSignedInteger(*raw, targetType, targetValue, strLenOrInd);
    }
    if (IsUnsignedIntegerTarget(targetType)) {
        const auto raw = GetAsUint64(parser, ydbType);
        if (!raw) return SQL_ERROR;
        return WriteUnsignedInteger(*raw, targetType, targetValue, strLenOrInd);
    }
    if (targetType == SQL_C_DOUBLE || targetType == SQL_C_FLOAT) {
        double value;
        if (ydbType == EPrimitiveType::Double) value = parser.GetDouble();
        else if (ydbType == EPrimitiveType::Float) value = parser.GetFloat();
        else if (const auto integer = GetAsInt64(parser, ydbType)) value = static_cast<double>(*integer);
        else if (const auto integer = GetAsUint64(parser, ydbType)) value = static_cast<double>(*integer);
        else return SQL_ERROR;
        if (targetType == SQL_C_DOUBLE) {
            if (targetValue) *static_cast<SQLDOUBLE*>(targetValue) = value;
            if (strLenOrInd) *strLenOrInd = sizeof(SQLDOUBLE);
        } else {
            if (targetValue) *static_cast<SQLREAL*>(targetValue) = static_cast<float>(value);
            if (strLenOrInd) *strLenOrInd = sizeof(SQLREAL);
        }
        return SQL_SUCCESS;
    }
    if (targetType == SQL_C_TYPE_DATE || targetType == SQL_C_DATE
        || targetType == SQL_C_TYPE_TIME || targetType == SQL_C_TIME
        || targetType == SQL_C_TYPE_TIMESTAMP || targetType == SQL_C_TIMESTAMP) {
        return WriteTemporal(parser, ydbType, targetType, targetValue, strLenOrInd);
    }
    if (targetType == SQL_C_BINARY) {
        if (ydbType != EPrimitiveType::String) return SQL_ERROR;
        const std::string& bytes = parser.GetString();
        return CopyVariable(bytes.data(), static_cast<SQLLEN>(bytes.size()), 0, 1,
                            targetValue, bufferLength, strLenOrInd, offset);
    }
    if (targetType != SQL_C_CHAR && targetType != SQL_C_WCHAR) return SQL_ERROR;

    std::string text;
    switch (ydbType) {
        case EPrimitiveType::Utf8: text = parser.GetUtf8(); break;
        case EPrimitiveType::String: text = parser.GetString(); break;
        case EPrimitiveType::Yson: text = parser.GetYson(); break;
        case EPrimitiveType::Json: text = parser.GetJson(); break;
        case EPrimitiveType::JsonDocument: text = parser.GetJsonDocument(); break;
        case EPrimitiveType::DyNumber: text = parser.GetDyNumber(); break;
        case EPrimitiveType::Uuid: text = parser.GetUuid().ToString(); break;
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
            text.assign(value.data(), value.size());
            if (parser.GetTimestamp().MicroSecondsOfSecond()) {
                char fraction[8] = {};
                std::snprintf(fraction, sizeof(fraction), ".%06u",
                              parser.GetTimestamp().MicroSecondsOfSecond());
                text += fraction;
            }
            break;
        }
        case EPrimitiveType::Timestamp64: {
            const auto micros = parser.GetTimestamp64().time_since_epoch().count();
            if (micros < 0) return SQL_ERROR;
            const TString value = TInstant::MicroSeconds(static_cast<ui64>(micros))
                .FormatGmTime("%Y-%m-%d %H:%M:%S");
            text.assign(value.data(), value.size());
            const auto fractionValue = static_cast<unsigned>(micros % 1000000);
            if (fractionValue) {
                char fraction[8] = {};
                std::snprintf(fraction, sizeof(fraction), ".%06u", fractionValue);
                text += fraction;
            }
            break;
        }
        case EPrimitiveType::TzDate: text = parser.GetTzDate(); break;
        case EPrimitiveType::TzDatetime: text = parser.GetTzDatetime(); break;
        case EPrimitiveType::TzTimestamp: text = parser.GetTzTimestamp(); break;
        default: return SQL_ERROR;
    }

    if (targetType == SQL_C_CHAR) {
        return CopyVariable(text.data(), static_cast<SQLLEN>(text.size()), 1, 1,
                            targetValue, bufferLength, strLenOrInd, offset);
    }
    try {
        const TUtf16String wide = UTF8ToWide(text);
        static_assert(sizeof(TUtf16String::value_type) == sizeof(SQLWCHAR));
        return CopyVariable(wide.data(), static_cast<SQLLEN>(wide.size() * sizeof(SQLWCHAR)),
                            sizeof(SQLWCHAR), sizeof(SQLWCHAR), targetValue, bufferLength,
                            strLenOrInd, offset);
    } catch (...) {
        SetInvalidCharacterValue();
        return SQL_ERROR;
    }
}

const char* ConsumeLastConvertSqlState() {
    const char* result = LastConvertSqlState;
    LastConvertSqlState = nullptr;
    return result;
}

} // namespace NYdb::NOdbc
