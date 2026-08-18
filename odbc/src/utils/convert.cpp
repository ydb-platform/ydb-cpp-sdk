#include "convert.h"
#include "sql_type_map.h"
#include "util.h"

#include <util/charset/wide.h>
#include <util/datetime/base.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace NYdb::NOdbc {
namespace {

thread_local const char* LastConvertSqlState = nullptr;

SQLRETURN Error(const char* state = nullptr) {
    LastConvertSqlState = state;
    return SQL_ERROR;
}

template <typename To, typename From>
std::optional<To> CheckedInteger(From value) {
    if (!std::in_range<To>(value)) {
        Error("22003");
        return std::nullopt;
    }
    return static_cast<To>(value);
}

template <typename To>
std::optional<To> ReadInteger(const TBoundParam& param) {
    if (!param.ParameterValuePtr) {
        return std::nullopt;
    }
    const auto value = VisitCInteger(param.ValueType, [&](auto type) {
        using T = typename decltype(type)::type;
        return CheckedInteger<To>(*static_cast<const T*>(param.ParameterValuePtr));
    });
    return value ? *value : std::nullopt;
}

template <typename To>
std::optional<To> ReadReal(const TBoundParam& param) {
    if (!param.ParameterValuePtr) {
        return std::nullopt;
    }
    if (param.ValueType == SQL_C_FLOAT) {
        return static_cast<To>(*static_cast<const SQLREAL*>(param.ParameterValuePtr));
    }
    if (param.ValueType == SQL_C_DOUBLE) {
        return static_cast<To>(*static_cast<const SQLDOUBLE*>(param.ParameterValuePtr));
    }
    return std::nullopt;
}

std::optional<std::string> ReadBytes(const TBoundParam& param) {
    if (!param.ParameterValuePtr) {
        return std::nullopt;
    }
    const char* data = static_cast<const char*>(param.ParameterValuePtr);
    SQLLEN length = param.StrLenOrIndPtr ? *param.StrLenOrIndPtr : param.BufferLength;
    if (length == SQL_NTS) {
        return std::string(data);
    }
    if (length < 0) {
        length = param.BufferLength;
    }
    if (length < 0) {
        return std::nullopt;
    }
    return std::string(data, static_cast<size_t>(length));
}

std::optional<std::string> ReadText(const TBoundParam& param) {
    if (param.ValueType == SQL_C_CHAR) {
        return ReadBytes(param);
    }
    if (param.ValueType != SQL_C_WCHAR || !param.ParameterValuePtr) {
        return std::nullopt;
    }
    const SQLLEN length = param.StrLenOrIndPtr ? *param.StrLenOrIndPtr : param.BufferLength;
    if (length == SQL_NTS) {
        return GetString(static_cast<SQLWCHAR*>(param.ParameterValuePtr), SQL_NTS);
    }
    if (length < 0 || length % static_cast<SQLLEN>(sizeof(SQLWCHAR))) {
        Error("22018");
        return std::nullopt;
    }
    return GetString(static_cast<SQLWCHAR*>(param.ParameterValuePtr),
                     static_cast<SQLINTEGER>(length / sizeof(SQLWCHAR)));
}

std::string ReadNumericStruct(const SQL_NUMERIC_STRUCT& numeric) {
    unsigned __int128 magnitude = 0;
    for (size_t i = 0; i < SQL_MAX_NUMERIC_LEN; ++i) {
        magnitude |= static_cast<unsigned __int128>(numeric.val[i]) << (i * 8);
    }
    std::string digits;
    do {
        digits.push_back(static_cast<char>('0' + magnitude % 10));
        magnitude /= 10;
    } while (magnitude);
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
    if (auto text = ReadText(param)) {
        return text;
    }
    if (param.ValueType == SQL_C_NUMERIC && param.ParameterValuePtr) {
        return ReadNumericStruct(*static_cast<const SQL_NUMERIC_STRUCT*>(param.ParameterValuePtr));
    }
    if (const auto value = ReadInteger<int64_t>(param)) {
        return std::to_string(*value);
    }
    std::ostringstream out;
    if (const auto value = ReadReal<double>(param)) {
        if (param.ValueType == SQL_C_FLOAT) {
            out << std::setprecision(std::numeric_limits<float>::max_digits10);
        } else {
            out << std::setprecision(std::numeric_limits<double>::max_digits10);
        }
        out << *value;
        return out.str();
    }
    return std::nullopt;
}

std::string FormatDecimalText(const TDecimalValue& decimal) {
    std::string text = decimal.ToString();
    const size_t point = text.find('.');
    const size_t fraction = point == std::string::npos ? 0 : text.size() - point - 1;
    if (decimal.DecimalType_.Scale > fraction) {
        if (point == std::string::npos) {
            text.push_back('.');
        }
        text.append(decimal.DecimalType_.Scale - fraction, '0');
    }
    return text;
}

std::optional<TInstant> ReadTemporal(const TBoundParam& param, EPrimitiveType type) {
    if (!param.ParameterValuePtr) {
        return std::nullopt;
    }
    char text[64] = {};
    if (type == EPrimitiveType::Date
        && (param.ValueType == SQL_C_TYPE_DATE || param.ValueType == SQL_C_DATE)) {
        const auto& v = *static_cast<const SQL_DATE_STRUCT*>(param.ParameterValuePtr);
        std::snprintf(text, sizeof(text), "%04d-%02u-%02uT00:00:00Z", v.year, v.month, v.day);
    } else if (type == EPrimitiveType::Datetime
               && (param.ValueType == SQL_C_TYPE_TIME || param.ValueType == SQL_C_TIME)) {
        const auto& v = *static_cast<const SQL_TIME_STRUCT*>(param.ParameterValuePtr);
        std::snprintf(text, sizeof(text), "1970-01-01T%02u:%02u:%02uZ",
                      v.hour, v.minute, v.second);
    } else if (type == EPrimitiveType::Timestamp
               && (param.ValueType == SQL_C_TYPE_TIMESTAMP || param.ValueType == SQL_C_TIMESTAMP)) {
        const auto& v = *static_cast<const SQL_TIMESTAMP_STRUCT*>(param.ParameterValuePtr);
        std::snprintf(text, sizeof(text), "%04d-%02u-%02uT%02u:%02u:%02u.%09uZ",
                      v.year, v.month, v.day, v.hour, v.minute, v.second, v.fraction);
    } else {
        Error("22007");
        return std::nullopt;
    }
    TInstant instant;
    if (!TInstant::TryParseIso8601(text, instant)) {
        Error("22007");
        return std::nullopt;
    }
    return instant;
}

SQLRETURN CopyVariable(const void* data, SQLLEN size, SQLLEN terminator, SQLLEN unit,
                       SQLPOINTER target, SQLLEN bufferLength, SQLLEN* indicator,
                       SQLLEN* offset) {
    if (offset && *offset < 0) {
        return SQL_NO_DATA;
    }
    const SQLLEN start = offset ? *offset : 0;
    if (start < 0 || start > size || start % unit) {
        return Error("HY090");
    }
    const SQLLEN remaining = size - start;
    SQLLEN copied = 0;
    if (target && bufferLength >= terminator) {
        SQLLEN capacity = bufferLength - terminator;
        capacity -= capacity % unit;
        copied = std::min(remaining, capacity);
        if (copied) {
            std::memcpy(target, static_cast<const char*>(data) + start, copied);
        }
        if (terminator) {
            std::memset(static_cast<char*>(target) + copied, 0, terminator);
        }
    }
    if (offset) {
        *offset = copied == remaining ? -1 : start + copied;
    }
    if (indicator) {
        *indicator = remaining;
    }
    return target && copied < remaining ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;
}

template <typename Value>
SQLRETURN WriteScalar(Value value, SQLPOINTER target, SQLLEN* indicator) {
    if (target) {
        *static_cast<Value*>(target) = value;
    }
    if (indicator) {
        *indicator = sizeof(Value);
    }
    return SQL_SUCCESS;
}

template <typename Target, typename Source>
SQLRETURN WriteCheckedInteger(Source value, SQLPOINTER target, SQLLEN* indicator) {
    const auto converted = CheckedInteger<Target>(value);
    return converted ? WriteScalar(*converted, target, indicator) : SQL_ERROR;
}

bool IsUnsignedIntegerTarget(SQLSMALLINT type) {
    return type == SQL_C_UTINYINT || type == SQL_C_USHORT
        || type == SQL_C_ULONG || type == SQL_C_UBIGINT;
}

bool IsIntegerTarget(SQLSMALLINT type) {
    return IsUnsignedIntegerTarget(type) || type == SQL_C_TINYINT || type == SQL_C_STINYINT
        || type == SQL_C_SHORT || type == SQL_C_SSHORT || type == SQL_C_LONG
        || type == SQL_C_SLONG || type == SQL_C_SBIGINT || type == SQL_C_BIT;
}

template <typename Source>
SQLRETURN WriteInteger(Source value, SQLSMALLINT type, SQLPOINTER target, SQLLEN* indicator) {
    if (type == SQL_C_BIT) {
        if (value != 0 && value != 1) {
            return Error("22003");
        }
        return WriteScalar<SQLCHAR>(value != 0, target, indicator);
    }
    return VisitCInteger(type, [&](auto targetType) {
        using T = typename decltype(targetType)::type;
        return WriteCheckedInteger<T>(value, target, indicator);
    }).value_or(SQL_ERROR);
}

SQLRETURN WriteReal(double value, SQLSMALLINT type, SQLPOINTER target, SQLLEN* indicator) {
    return type == SQL_C_DOUBLE
        ? WriteScalar<SQLDOUBLE>(value, target, indicator)
        : WriteScalar<SQLREAL>(static_cast<SQLREAL>(value), target, indicator);
}

std::optional<TInstant> GetAsInstant(TValueParser& parser, EPrimitiveType type) {
    switch (type) {
        case EPrimitiveType::Date: return parser.GetDate();
        case EPrimitiveType::Datetime: return parser.GetDatetime();
        case EPrimitiveType::Timestamp: return parser.GetTimestamp();
        case EPrimitiveType::Date32: {
            const auto value = parser.GetDate32().time_since_epoch().count();
            if (value >= 0) {
                return TInstant::Days(static_cast<uint64_t>(value));
            }
            break;
        }
        case EPrimitiveType::Datetime64: {
            const auto value = parser.GetDatetime64().time_since_epoch().count();
            if (value >= 0) {
                return TInstant::Seconds(static_cast<uint64_t>(value));
            }
            break;
        }
        case EPrimitiveType::Timestamp64: {
            const auto value = parser.GetTimestamp64().time_since_epoch().count();
            if (value >= 0) {
                return TInstant::MicroSeconds(static_cast<uint64_t>(value));
            }
            break;
        }
        case EPrimitiveType::TzDate:
        case EPrimitiveType::TzDatetime:
        case EPrimitiveType::TzTimestamp: {
            const std::string value = type == EPrimitiveType::TzDate ? parser.GetTzDate()
                : type == EPrimitiveType::TzDatetime ? parser.GetTzDatetime()
                                                     : parser.GetTzTimestamp();
            TInstant instant;
            if (TInstant::TryParseIso8601(value, instant)) {
                return instant;
            }
            break;
        }
        default: return std::nullopt;
    }
    Error("22007");
    return std::nullopt;
}

SQLRETURN WriteTemporal(TValueParser& parser, EPrimitiveType type, SQLSMALLINT targetType,
                        SQLPOINTER target, SQLLEN* indicator) {
    const auto instant = GetAsInstant(parser, type);
    struct tm value = {};
    if (!instant) {
        return SQL_ERROR;
    }
    if (!instant->GmTime(&value)) {
        return Error("22007");
    }
    if (targetType == SQL_C_TYPE_DATE || targetType == SQL_C_DATE) {
        SQL_DATE_STRUCT out{static_cast<SQLSMALLINT>(value.tm_year + 1900),
                            static_cast<SQLUSMALLINT>(value.tm_mon + 1),
                            static_cast<SQLUSMALLINT>(value.tm_mday)};
        return WriteScalar(out, target, indicator);
    }
    if (targetType == SQL_C_TYPE_TIME || targetType == SQL_C_TIME) {
        SQL_TIME_STRUCT out{static_cast<SQLUSMALLINT>(value.tm_hour),
                            static_cast<SQLUSMALLINT>(value.tm_min),
                            static_cast<SQLUSMALLINT>(value.tm_sec)};
        return WriteScalar(out, target, indicator);
    }
    SQL_TIMESTAMP_STRUCT out{static_cast<SQLSMALLINT>(value.tm_year + 1900),
                             static_cast<SQLUSMALLINT>(value.tm_mon + 1),
                             static_cast<SQLUSMALLINT>(value.tm_mday),
                             static_cast<SQLUSMALLINT>(value.tm_hour),
                             static_cast<SQLUSMALLINT>(value.tm_min),
                             static_cast<SQLUSMALLINT>(value.tm_sec),
                             instant->MicroSecondsOfSecond() * 1000};
    return WriteScalar(out, target, indicator);
}

SQLRETURN WriteNumericStruct(std::string text, SQLPOINTER target, SQLLEN* indicator,
                             SQLULEN precision, SQLSMALLINT scale) {
    bool positive = true;
    if (!text.empty() && (text.front() == '-' || text.front() == '+')) {
        positive = text.front() != '-';
        text.erase(text.begin());
    }
    const auto point = text.find('.');
    if (point != std::string::npos) {
        text.erase(point, 1);
    }
    while (!text.empty() && text.front() == '0') {
        text.erase(text.begin());
    }
    if (text.empty()) {
        text = "0";
    }
    unsigned __int128 magnitude = 0;
    for (const char ch : text) {
        if (ch < '0' || ch > '9') {
            return Error("22018");
        }
        const auto previous = magnitude;
        magnitude = magnitude * 10 + static_cast<unsigned>(ch - '0');
        if (magnitude < previous) {
            return Error("22003");
        }
    }
    if (target) {
        auto& out = *static_cast<SQL_NUMERIC_STRUCT*>(target);
        std::memset(&out, 0, sizeof(out));
        out.precision = static_cast<SQLCHAR>(precision);
        out.scale = static_cast<SQLSCHAR>(scale);
        out.sign = positive ? 1 : 0;
        for (size_t i = 0; i < SQL_MAX_NUMERIC_LEN; ++i) {
            out.val[i] = static_cast<SQLCHAR>(magnitude & 0xff);
            magnitude >>= 8;
        }
        if (magnitude) {
            return Error("22003");
        }
    }
    if (indicator) {
        *indicator = sizeof(SQL_NUMERIC_STRUCT);
    }
    return SQL_SUCCESS;
}

SQLRETURN WriteText(std::string_view text, SQLSMALLINT type, SQLPOINTER target,
                    SQLLEN bufferLength, SQLLEN* indicator, SQLLEN* offset) {
    if (type == SQL_C_CHAR) {
        return CopyVariable(text.data(), text.size(), 1, 1, target, bufferLength, indicator, offset);
    }
    if (type != SQL_C_WCHAR) {
        return SQL_ERROR;
    }
    try {
        const TUtf16String wide = UTF8ToWide(text);
        static_assert(sizeof(TUtf16String::value_type) == sizeof(SQLWCHAR));
        return CopyVariable(wide.data(), wide.size() * sizeof(SQLWCHAR), sizeof(SQLWCHAR),
                            sizeof(SQLWCHAR), target, bufferLength, indicator, offset);
    } catch (...) {
        return Error("22018");
    }
}

template <typename Integer>
std::optional<Integer> ParseDecimalInteger(const std::string& text) {
    try {
        const std::string integral = text.substr(0, text.find('.'));
        if constexpr (std::is_unsigned_v<Integer>) {
            if (!integral.empty() && integral.front() == '-') {
                throw std::out_of_range("negative");
            }
        }
        size_t parsed = 0;
        const Integer value = std::is_unsigned_v<Integer>
            ? static_cast<Integer>(std::stoull(integral, &parsed))
            : static_cast<Integer>(std::stoll(integral, &parsed));
        if (parsed != integral.size()) {
            throw std::invalid_argument("decimal");
        }
        return value;
    } catch (...) {
        Error("22003");
        return std::nullopt;
    }
}

SQLRETURN ConvertDecimal(TValueParser& parser, SQLSMALLINT type, SQLPOINTER target,
                         SQLLEN bufferLength, SQLLEN* indicator, SQLLEN* offset) {
    const TDecimalValue decimal = parser.GetDecimal();
    const std::string text = FormatDecimalText(decimal);
    if (type == SQL_C_DOUBLE || type == SQL_C_FLOAT) {
        try {
            return WriteReal(std::stod(text), type, target, indicator);
        } catch (...) {
            return Error("22003");
        }
    }
    if (type == SQL_C_NUMERIC) {
        return WriteNumericStruct(text, target, indicator, decimal.DecimalType_.Precision,
                                  decimal.DecimalType_.Scale);
    }
    if (IsIntegerTarget(type)) {
        if (IsUnsignedIntegerTarget(type)) {
            const auto value = ParseDecimalInteger<uint64_t>(text);
            return value ? WriteInteger(*value, type, target, indicator) : SQL_ERROR;
        }
        const auto value = ParseDecimalInteger<int64_t>(text);
        return value ? WriteInteger(*value, type, target, indicator) : SQL_ERROR;
    }
    return WriteText(text, type, target, bufferLength, indicator, offset);
}

template <typename Value>
TOdbcScalar MakeScalar(Value value) {
    if constexpr (std::is_floating_point_v<Value>) {
        return double(value);
    } else if constexpr (std::is_unsigned_v<Value>) {
        return uint64_t(value);
    } else {
        return int64_t(value);
    }
}

template <typename Value>
std::optional<Value> ReadScalar(const TBoundParam& param) {
    if constexpr (std::is_floating_point_v<Value>) {
        return ReadReal<Value>(param);
    } else {
        return ReadInteger<Value>(param);
    }
}

template <typename Result, typename Fn>
std::optional<Result> VisitScalar(EPrimitiveType type, Fn&& fn) {
#define ODBC_VISIT_SCALAR(name, cppType, sqlType, isUnsigned)                      \
    case EPrimitiveType::name:                                                    \
        return fn.template operator()<cppType>(                                   \
            [](TValueParser& parser) { return parser.Get##name(); },              \
            [](TParamValueBuilder& builder, cppType value) {                      \
                builder.Optional##name(value);                                    \
            });
    switch (type) {
        YDB_ODBC_SCALAR_TYPES(ODBC_VISIT_SCALAR)
        default:
            return std::nullopt;
    }
#undef ODBC_VISIT_SCALAR
}

std::optional<TOdbcScalar> PrimitiveScalar(TValueParser& parser, EPrimitiveType type) {
    if (auto scalar = VisitScalar<TOdbcScalar>(type, [&]<typename T>(auto get, auto) {
            return MakeScalar(get(parser));
        })) {
        return *scalar;
    }
    switch (type) {
        case EPrimitiveType::Bool: return TOdbcScalar{int64_t(parser.GetBool())};
        case EPrimitiveType::Utf8: return TOdbcScalar{parser.GetUtf8()};
        case EPrimitiveType::String: return TOdbcScalar{parser.GetString()};
        case EPrimitiveType::Yson: return TOdbcScalar{parser.GetYson()};
        case EPrimitiveType::Json: return TOdbcScalar{parser.GetJson()};
        case EPrimitiveType::JsonDocument: return TOdbcScalar{parser.GetJsonDocument()};
        case EPrimitiveType::DyNumber: return TOdbcScalar{parser.GetDyNumber()};
        case EPrimitiveType::Uuid: return TOdbcScalar{parser.GetUuid().ToString()};
        default: return std::nullopt;
    }
}

std::string FormatInstant(TInstant value, const char* format, bool fraction) {
    const TString formatted = value.FormatGmTime(format);
    std::string text(formatted.data(), formatted.size());
    if (fraction && value.MicroSecondsOfSecond()) {
        char suffix[8] = {};
        std::snprintf(suffix, sizeof(suffix), ".%06u", value.MicroSecondsOfSecond());
        text += suffix;
    }
    return text;
}

std::optional<std::string> TemporalText(TValueParser& parser, EPrimitiveType type) {
    switch (type) {
        case EPrimitiveType::Date:
            return FormatInstant(parser.GetDate(), "%Y-%m-%d", false);
        case EPrimitiveType::Datetime:
            return FormatInstant(parser.GetDatetime(), "%Y-%m-%d %H:%M:%S", false);
        case EPrimitiveType::Timestamp:
            return FormatInstant(parser.GetTimestamp(), "%Y-%m-%d %H:%M:%S", true);
        case EPrimitiveType::Date32: {
            const auto value = parser.GetDate32().time_since_epoch().count();
            return value < 0 ? std::nullopt
                             : std::optional<std::string>(FormatInstant(
                                   TInstant::Days(value), "%Y-%m-%d", false));
        }
        case EPrimitiveType::Datetime64: {
            const auto value = parser.GetDatetime64().time_since_epoch().count();
            return value < 0 ? std::nullopt
                             : std::optional<std::string>(FormatInstant(
                                   TInstant::Seconds(value), "%Y-%m-%d %H:%M:%S", false));
        }
        case EPrimitiveType::Timestamp64: {
            const auto value = parser.GetTimestamp64().time_since_epoch().count();
            return value < 0 ? std::nullopt
                             : std::optional<std::string>(FormatInstant(
                                   TInstant::MicroSeconds(value), "%Y-%m-%d %H:%M:%S", true));
        }
        case EPrimitiveType::TzDate: return parser.GetTzDate();
        case EPrimitiveType::TzDatetime: return parser.GetTzDatetime();
        case EPrimitiveType::TzTimestamp: return parser.GetTzTimestamp();
        default: return std::nullopt;
    }
}

template <typename Value, typename Put>
bool PutValue(std::optional<Value> value, Put put) {
    if (!value) {
        return false;
    }
    put(*value);
    return true;
}

bool ConvertParamValue(const TBoundParam& param, EPrimitiveType type, TParamValueBuilder& builder) {
    if (auto converted = VisitScalar<bool>(type, [&]<typename T>(auto, auto put) {
            return PutValue(ReadScalar<T>(param), [&](T value) {
                put(builder, value);
            });
        })) {
        return *converted;
    }
    switch (type) {
        case EPrimitiveType::Bool:
            if (const auto value = ReadInteger<int64_t>(param); value && *value >= 0 && *value <= 1) {
                builder.OptionalBool(*value != 0);
                return true;
            }
            Error("22003");
            return false;
        case EPrimitiveType::Utf8:
            return PutValue(ReadText(param), [&](const auto& v) { builder.OptionalUtf8(v); });
        case EPrimitiveType::String: {
            if (param.ValueType != SQL_C_BINARY) {
                return false;
            }
            const auto value = ReadBytes(param);
            if (value) {
                builder.OptionalString(*value);
            }
            return value.has_value();
        }
        case EPrimitiveType::Date:
        case EPrimitiveType::Datetime:
        case EPrimitiveType::Timestamp: {
            const auto value = ReadTemporal(param, type);
            if (!value) {
                return false;
            }
            if (type == EPrimitiveType::Date) {
                builder.OptionalDate(*value);
            } else if (type == EPrimitiveType::Datetime) {
                builder.OptionalDatetime(*value);
            } else {
                builder.OptionalTimestamp(*value);
            }
            return true;
        }
        default: return false;
    }
}

} // namespace

SQLRETURN ConvertParam(const TBoundParam& param, TParamValueBuilder& builder) {
    LastConvertSqlState = nullptr;
    const auto type = ResolveParamType(param);
    if (!type) {
        return SQL_ERROR;
    }
    if (param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA) {
        TTypeBuilder itemType;
        if (!type->Type) {
            itemType.Decimal(TDecimalType(static_cast<uint8_t>(type->Precision),
                                          static_cast<uint8_t>(type->Scale)));
        } else {
            itemType.Primitive(*type->Type);
        }
        builder.EmptyOptional(itemType.Build()).Build();
        return SQL_SUCCESS;
    }
    if (!type->Type) {
        const auto text = ReadDecimalText(param);
        if (!text) {
            return SQL_ERROR;
        }
        try {
            builder.BeginOptional()
                .Decimal(TDecimalValue(*text, static_cast<uint8_t>(type->Precision),
                                       static_cast<uint8_t>(type->Scale)))
                .EndOptional();
        } catch (...) {
            return Error("22018");
        }
        builder.Build();
        return SQL_SUCCESS;
    }
    if (!ConvertParamValue(param, *type->Type, builder)) {
        return SQL_ERROR;
    }
    builder.Build();
    return SQL_SUCCESS;
}

SQLRETURN ConvertColumn(const TOdbcScalar& value, SQLSMALLINT targetType, SQLPOINTER targetValue,
                        SQLLEN bufferLength, SQLLEN* strLenOrInd, SQLLEN* offset) {
    LastConvertSqlState = nullptr;
    if (bufferLength < 0) {
        return Error("HY090");
    }
    if (std::holds_alternative<std::monostate>(value)) {
        if (!strLenOrInd) {
            return Error("22002");
        }
        *strLenOrInd = SQL_NULL_DATA;
        return SQL_SUCCESS;
    }
    return std::visit(
        [&](const auto& scalar) -> SQLRETURN {
            using T = std::decay_t<decltype(scalar)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return WriteText(
                    scalar, targetType, targetValue, bufferLength, strLenOrInd, offset);
            } else if constexpr (std::is_same_v<T, std::monostate>) {
                return SQL_ERROR;
            } else {
                if constexpr (!std::is_floating_point_v<T>) {
                    if (IsIntegerTarget(targetType)) {
                        return WriteInteger(scalar, targetType, targetValue, strLenOrInd);
                    }
                }
                if (targetType == SQL_C_DOUBLE || targetType == SQL_C_FLOAT) {
                    return WriteReal(scalar, targetType, targetValue, strLenOrInd);
                }
                if (targetType == SQL_C_CHAR || targetType == SQL_C_WCHAR) {
                    return WriteText(std::to_string(scalar), targetType, targetValue, bufferLength,
                                     strLenOrInd, offset);
                }
                return SQL_ERROR;
            }
        },
        value);
}

SQLRETURN ConvertColumn(TValueParser& parser, SQLSMALLINT targetType, SQLPOINTER targetValue,
                        SQLLEN bufferLength, SQLLEN* strLenOrInd, SQLLEN* offset) {
    LastConvertSqlState = nullptr;
    if (bufferLength < 0) {
        return Error("HY090");
    }
    if (parser.IsNull()) {
        if (!strLenOrInd) {
            return Error("22002");
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
        return ConvertDecimal(parser, targetType, targetValue, bufferLength, strLenOrInd, offset);
    }
    if (parser.GetKind() != TTypeParser::ETypeKind::Primitive) {
        return SQL_ERROR;
    }
    const EPrimitiveType type = parser.GetPrimitiveType();
    if (targetType == SQL_C_TYPE_DATE || targetType == SQL_C_DATE
        || targetType == SQL_C_TYPE_TIME || targetType == SQL_C_TIME
        || targetType == SQL_C_TYPE_TIMESTAMP || targetType == SQL_C_TIMESTAMP) {
        return WriteTemporal(parser, type, targetType, targetValue, strLenOrInd);
    }
    if (targetType == SQL_C_BINARY) {
        if (type != EPrimitiveType::String) {
            return SQL_ERROR;
        }
        const std::string& bytes = parser.GetString();
        return CopyVariable(bytes.data(), bytes.size(), 0, 1, targetValue, bufferLength,
                            strLenOrInd, offset);
    }
    if (const auto scalar = PrimitiveScalar(parser, type)) {
        return ConvertColumn(*scalar, targetType, targetValue, bufferLength, strLenOrInd, offset);
    }
    const auto text = TemporalText(parser, type);
    return text ? WriteText(*text, targetType, targetValue, bufferLength, strLenOrInd, offset)
                : SQL_ERROR;
}

const char* ConsumeLastConvertSqlState() {
    const char* result = LastConvertSqlState;
    LastConvertSqlState = nullptr;
    return result;
}

} // namespace NYdb::NOdbc
