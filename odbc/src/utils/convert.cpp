#include "convert.h"

#include <util/datetime/base.h>
#include <util/generic/singleton.h>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <map>
#include <optional>

namespace NYdb {
namespace NOdbc {

namespace {

thread_local const char* LastConvertSqlState = nullptr;

bool FitsInt16(int64_t value) {
    return value >= INT16_MIN && value <= INT16_MAX;
}

bool FitsInt32(int64_t value) {
    return value >= INT32_MIN && value <= INT32_MAX;
}

void SetNumericOutOfRange() {
    LastConvertSqlState = "22003";
}

std::optional<int64_t> GetAsInt64(TValueParser& parser, EPrimitiveType ydbType) {
    switch (ydbType) {
        case EPrimitiveType::Int8: return parser.GetInt8();
        case EPrimitiveType::Uint8: return parser.GetUint8();
        case EPrimitiveType::Int16: return parser.GetInt16();
        case EPrimitiveType::Uint16: return parser.GetUint16();
        case EPrimitiveType::Int32: return parser.GetInt32();
        case EPrimitiveType::Uint32: return parser.GetUint32();
        case EPrimitiveType::Int64: return parser.GetInt64();
        case EPrimitiveType::Uint64: {
            const uint64_t unsignedValue = parser.GetUint64();
            if (unsignedValue > static_cast<uint64_t>(INT64_MAX)) {
                SetNumericOutOfRange();
                return std::nullopt;
            }
            return static_cast<int64_t>(unsignedValue);
        }
        case EPrimitiveType::Bool: return parser.GetBool() ? 1 : 0;
        default: return std::nullopt;
    }
}

} // namespace

template<SQLSMALLINT CType>
struct TSqlTypeTraits;

template<> struct TSqlTypeTraits<SQL_C_CHAR> { using Type = std::string; };
template<> struct TSqlTypeTraits<SQL_C_BINARY> { using Type = std::string; };
template<> struct TSqlTypeTraits<SQL_C_SBIGINT> { using Type = SQLBIGINT; };
template<> struct TSqlTypeTraits<SQL_C_UBIGINT> { using Type = SQLUBIGINT; };
template<> struct TSqlTypeTraits<SQL_C_LONG> { using Type = SQLINTEGER; };
template<> struct TSqlTypeTraits<SQL_C_SLONG> { using Type = SQLINTEGER; };
template<> struct TSqlTypeTraits<SQL_C_ULONG> { using Type = SQLUINTEGER; };
template<> struct TSqlTypeTraits<SQL_C_SHORT> { using Type = SQLSMALLINT; };
template<> struct TSqlTypeTraits<SQL_C_SSHORT> { using Type = SQLSMALLINT; };
template<> struct TSqlTypeTraits<SQL_C_USHORT> { using Type = SQLUSMALLINT; };
template<> struct TSqlTypeTraits<SQL_C_TINYINT> { using Type = SQLSCHAR; };
template<> struct TSqlTypeTraits<SQL_C_UTINYINT> { using Type = SQLCHAR; };
template<> struct TSqlTypeTraits<SQL_C_DOUBLE> { using Type = SQLDOUBLE; };
template<> struct TSqlTypeTraits<SQL_C_FLOAT> { using Type = SQLFLOAT; };
template<> struct TSqlTypeTraits<SQL_C_BIT> { using Type = SQLCHAR; };

template<SQLSMALLINT CType>
struct TTypedValue {
    using TSrcType = typename TSqlTypeTraits<CType>::Type;
    
    TSrcType Data;

    TTypedValue(const TBoundParam& param) {
        Data = *static_cast<const TSrcType*>(param.ParameterValuePtr);
    }
};

template<>
TTypedValue<SQL_C_CHAR>::TTypedValue(const TBoundParam& param) {
    if (param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA) {
        Data.clear();
        return;
    }
    
    const char* ptr = static_cast<const char*>(param.ParameterValuePtr);
    if (!ptr) {
        Data.clear();
        return;
    }
    
    if (param.StrLenOrIndPtr) {
        SQLLEN len = *param.StrLenOrIndPtr;
        if (len == SQL_NTS) {
            Data = std::string(ptr);
        } else if (len >= 0) {
            Data = std::string(ptr, static_cast<size_t>(len));
        } else {
            Data = std::string(ptr, param.BufferLength);
        }
    } else {
        Data = std::string(ptr, param.BufferLength);
    }
}

template<>
TTypedValue<SQL_C_BINARY>::TTypedValue(const TBoundParam& param) {
    if (param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA) {
        Data.clear();
        return;
    }
    
    const char* ptr = static_cast<const char*>(param.ParameterValuePtr);
    if (!ptr) {
        Data.clear();
        return;
    }
    
    if (param.StrLenOrIndPtr && *param.StrLenOrIndPtr >= 0) {
        Data = std::string(ptr, static_cast<size_t>(*param.StrLenOrIndPtr));
    } else {
        Data = std::string(ptr, param.BufferLength);
    }
}

class IConverter {
public:
    virtual void AddToBuilder(const TBoundParam& param, TParamValueBuilder& builder) = 0;

    virtual ~IConverter() = default;
};

template<SQLSMALLINT CType, SQLSMALLINT SqlType>
class TConverter : public IConverter {
public:
    virtual void AddToBuilder(const TBoundParam& param, TParamValueBuilder& builder) override {
        TTypedValue<CType> value(param);
        Convert(param, std::move(value.Data), builder);
        if (param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA) {
            builder.EmptyOptional(GetType());
        }
        builder.Build();
    }

private:
    void Convert(const TBoundParam& param, TTypedValue<CType>::TSrcType&& data, TParamValueBuilder& builder);
    TType GetType();
};

class TConverterRegistry {
public:
    static TConverterRegistry& GetInstance() {
        static TConverterRegistry instance;
        return instance;
    }

    void RegisterConverter(SQLSMALLINT cType, SQLSMALLINT sqlType, std::unique_ptr<IConverter> converter) {
        Converters_.emplace(std::make_pair(cType, sqlType), std::move(converter));
    }

    IConverter* GetConverter(SQLSMALLINT cType, SQLSMALLINT sqlType) {
        auto it = Converters_.find(std::make_pair(cType, sqlType));
        if (it != Converters_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

private:
    std::map<std::pair<SQLSMALLINT, SQLSMALLINT>, std::unique_ptr<IConverter>> Converters_;
};

#define REGISTER_CONVERTER(CType, SqlType, YdbType) \
    struct TConverterRegistration##CType##SqlType { \
        TConverterRegistration##CType##SqlType() { \
            TConverterRegistry::GetInstance().RegisterConverter(CType, SqlType, std::make_unique<TConverter<CType, SqlType>>()); \
        } \
    }; \
    static const TConverterRegistration##CType##SqlType converterRegistration##CType##SqlType; \
    template<> \
    TType TConverter<CType, SqlType>::GetType() { \
        return TTypeBuilder().Primitive(YdbType).Build(); \
    } \
    template<> \
    void TConverter<CType, SqlType>::Convert(const TBoundParam& param, TTypedValue<CType>::TSrcType&& data, TParamValueBuilder& builder)

// Integer types

REGISTER_CONVERTER(SQL_C_SBIGINT, SQL_BIGINT, EPrimitiveType::Int64) {
    builder.OptionalInt64(static_cast<std::int64_t>(data));
}

REGISTER_CONVERTER(SQL_C_LONG, SQL_BIGINT, EPrimitiveType::Int64) {
    builder.OptionalInt64(static_cast<std::int64_t>(data));
}

REGISTER_CONVERTER(SQL_C_SLONG, SQL_BIGINT, EPrimitiveType::Int64) {
    builder.OptionalInt64(static_cast<std::int64_t>(data));
}

REGISTER_CONVERTER(SQL_C_SHORT, SQL_BIGINT, EPrimitiveType::Int64) {
    builder.OptionalInt64(static_cast<std::int64_t>(data));
}

REGISTER_CONVERTER(SQL_C_TINYINT, SQL_BIGINT, EPrimitiveType::Int64) {
    builder.OptionalInt64(static_cast<std::int64_t>(data));
}

REGISTER_CONVERTER(SQL_C_UBIGINT, SQL_BIGINT, EPrimitiveType::Uint64) {
    builder.OptionalUint64(static_cast<std::uint64_t>(data));
}

REGISTER_CONVERTER(SQL_C_ULONG, SQL_BIGINT, EPrimitiveType::Uint64) {
    builder.OptionalUint64(static_cast<std::uint64_t>(data));
}

REGISTER_CONVERTER(SQL_C_USHORT, SQL_BIGINT, EPrimitiveType::Uint64) {
    builder.OptionalUint64(static_cast<std::uint64_t>(data));
}

REGISTER_CONVERTER(SQL_C_UTINYINT, SQL_BIGINT, EPrimitiveType::Uint64) {
    builder.OptionalUint64(static_cast<std::uint64_t>(data));
}

REGISTER_CONVERTER(SQL_C_SBIGINT, SQL_INTEGER, EPrimitiveType::Int32) {
    builder.OptionalInt32(static_cast<std::int32_t>(data));
}

REGISTER_CONVERTER(SQL_C_LONG, SQL_INTEGER, EPrimitiveType::Int32) {
    builder.OptionalInt32(static_cast<std::int32_t>(data));
}

REGISTER_CONVERTER(SQL_C_SLONG, SQL_INTEGER, EPrimitiveType::Int32) {
    builder.OptionalInt32(static_cast<std::int32_t>(data));
}

REGISTER_CONVERTER(SQL_C_SHORT, SQL_INTEGER, EPrimitiveType::Int32) {
    builder.OptionalInt32(static_cast<std::int32_t>(data));
}

REGISTER_CONVERTER(SQL_C_TINYINT, SQL_INTEGER, EPrimitiveType::Int32) {
    builder.OptionalInt32(static_cast<std::int32_t>(data));
}

REGISTER_CONVERTER(SQL_C_UBIGINT, SQL_INTEGER, EPrimitiveType::Uint32) {
    builder.OptionalUint32(static_cast<std::uint32_t>(data));
}

REGISTER_CONVERTER(SQL_C_ULONG, SQL_INTEGER, EPrimitiveType::Uint32) {
    builder.OptionalUint32(static_cast<std::uint32_t>(data));
}

REGISTER_CONVERTER(SQL_C_USHORT, SQL_INTEGER, EPrimitiveType::Uint32) {
    builder.OptionalUint32(static_cast<std::uint32_t>(data));
}

REGISTER_CONVERTER(SQL_C_UTINYINT, SQL_INTEGER, EPrimitiveType::Uint32) {
    builder.OptionalUint32(static_cast<std::uint32_t>(data));
}

REGISTER_CONVERTER(SQL_C_SBIGINT, SQL_SMALLINT, EPrimitiveType::Int16) {
    builder.OptionalInt16(static_cast<std::int16_t>(data));
}

REGISTER_CONVERTER(SQL_C_LONG, SQL_SMALLINT, EPrimitiveType::Int16) {
    builder.OptionalInt16(static_cast<std::int16_t>(data));
}

REGISTER_CONVERTER(SQL_C_SLONG, SQL_SMALLINT, EPrimitiveType::Int16) {
    builder.OptionalInt16(static_cast<std::int16_t>(data));
}

REGISTER_CONVERTER(SQL_C_SHORT, SQL_SMALLINT, EPrimitiveType::Int16) {
    builder.OptionalInt16(static_cast<std::int16_t>(data));
}

REGISTER_CONVERTER(SQL_C_TINYINT, SQL_SMALLINT, EPrimitiveType::Int16) {
    builder.OptionalInt16(static_cast<std::int16_t>(data));
}

REGISTER_CONVERTER(SQL_C_UBIGINT, SQL_SMALLINT, EPrimitiveType::Uint16) {
    builder.OptionalUint16(static_cast<std::uint16_t>(data));
}

REGISTER_CONVERTER(SQL_C_ULONG, SQL_SMALLINT, EPrimitiveType::Uint16) {
    builder.OptionalUint16(static_cast<std::uint16_t>(data));
}

REGISTER_CONVERTER(SQL_C_USHORT, SQL_SMALLINT, EPrimitiveType::Uint16) {
    builder.OptionalUint16(static_cast<std::uint16_t>(data));
}

REGISTER_CONVERTER(SQL_C_UTINYINT, SQL_SMALLINT, EPrimitiveType::Uint16) {
    builder.OptionalUint16(static_cast<std::uint16_t>(data));
}

REGISTER_CONVERTER(SQL_C_SBIGINT, SQL_TINYINT, EPrimitiveType::Int8) {
    builder.OptionalInt8(static_cast<std::int8_t>(data));
}

REGISTER_CONVERTER(SQL_C_LONG, SQL_TINYINT, EPrimitiveType::Int8) {
    builder.OptionalInt8(static_cast<std::int8_t>(data));
}

REGISTER_CONVERTER(SQL_C_SLONG, SQL_TINYINT, EPrimitiveType::Int8) {
    builder.OptionalInt8(static_cast<std::int8_t>(data));
}

REGISTER_CONVERTER(SQL_C_SHORT, SQL_TINYINT, EPrimitiveType::Int8) {
    builder.OptionalInt8(static_cast<std::int8_t>(data));
}

REGISTER_CONVERTER(SQL_C_TINYINT, SQL_TINYINT, EPrimitiveType::Int8) {
    builder.OptionalInt8(static_cast<std::int8_t>(data));
}

REGISTER_CONVERTER(SQL_C_UBIGINT, SQL_TINYINT, EPrimitiveType::Uint8) {
    builder.OptionalUint8(static_cast<std::uint8_t>(data));
}

REGISTER_CONVERTER(SQL_C_ULONG, SQL_TINYINT, EPrimitiveType::Uint8) {
    builder.OptionalUint8(static_cast<std::uint8_t>(data));
}

REGISTER_CONVERTER(SQL_C_USHORT, SQL_TINYINT, EPrimitiveType::Uint8) {
    builder.OptionalUint8(static_cast<std::uint8_t>(data));
}

REGISTER_CONVERTER(SQL_C_UTINYINT, SQL_TINYINT, EPrimitiveType::Uint8) {
    builder.OptionalUint8(static_cast<std::uint8_t>(data));
}

// Floating point types

REGISTER_CONVERTER(SQL_C_FLOAT, SQL_REAL, EPrimitiveType::Float) {
    builder.OptionalFloat(data);
}

REGISTER_CONVERTER(SQL_C_DOUBLE, SQL_FLOAT, EPrimitiveType::Double) {
    builder.OptionalDouble(data);
}

REGISTER_CONVERTER(SQL_C_DOUBLE, SQL_DOUBLE, EPrimitiveType::Double) {
    builder.OptionalDouble(data);
}

// String types

REGISTER_CONVERTER(SQL_C_CHAR, SQL_CHAR, EPrimitiveType::Utf8) {
    builder.OptionalUtf8(std::move(data));
}

REGISTER_CONVERTER(SQL_C_CHAR, SQL_VARCHAR, EPrimitiveType::Utf8) {
    builder.OptionalUtf8(std::move(data));
}

REGISTER_CONVERTER(SQL_C_CHAR, SQL_LONGVARCHAR, EPrimitiveType::Utf8) {
    builder.OptionalUtf8(std::move(data));
}

// Binary types

REGISTER_CONVERTER(SQL_C_BINARY, SQL_BINARY, EPrimitiveType::String) {
    builder.OptionalString(std::move(data));
}

REGISTER_CONVERTER(SQL_C_BINARY, SQL_VARBINARY, EPrimitiveType::String) {
    builder.OptionalString(std::move(data));
}

REGISTER_CONVERTER(SQL_C_BINARY, SQL_LONGVARBINARY, EPrimitiveType::String) {
    builder.OptionalString(std::move(data));
}

#undef REGISTER_CONVERTER

SQLRETURN ConvertParam(const TBoundParam& param, TParamValueBuilder& builder) {
    auto converter = TConverterRegistry::GetInstance().GetConverter(param.ValueType, param.ParameterType);
    if (!converter) {
        return SQL_ERROR;
    }

    converter->AddToBuilder(param, builder);
    return SQL_SUCCESS;
}

SQLRETURN ConvertColumn(TValueParser& parser, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) {
    LastConvertSqlState = nullptr;
    if (parser.IsNull()) {
        if (strLenOrInd) {
            *strLenOrInd = SQL_NULL_DATA;
        }
        return SQL_SUCCESS;
    }

    if (parser.GetKind() == TTypeParser::ETypeKind::Optional) {
        parser.OpenOptional();
        SQLRETURN ret = ConvertColumn(parser, targetType, targetValue, bufferLength, strLenOrInd);
        parser.CloseOptional();
        return ret;
    }

    if (parser.GetKind() != TTypeParser::ETypeKind::Primitive) {
        return SQL_ERROR;
    }

    EPrimitiveType ydbType = parser.GetPrimitiveType();

    switch (targetType) {
        case SQL_C_SHORT:
        case SQL_C_SSHORT:
        {
            const auto raw = GetAsInt64(parser, ydbType);
            if (!raw) {
                return SQL_ERROR;
            }
            if (!FitsInt16(*raw)) {
                SetNumericOutOfRange();
                return SQL_ERROR;
            }
            if (targetValue) {
                *reinterpret_cast<SQLSMALLINT*>(targetValue) = static_cast<SQLSMALLINT>(*raw);
            }
            if (strLenOrInd) {
                *strLenOrInd = sizeof(SQLSMALLINT);
            }
            return SQL_SUCCESS;
        }
        case SQL_C_SLONG:
        case SQL_C_LONG:
        {
            const auto raw = GetAsInt64(parser, ydbType);
            if (!raw) {
                return SQL_ERROR;
            }
            if (!FitsInt32(*raw)) {
                SetNumericOutOfRange();
                return SQL_ERROR;
            }
            if (targetValue) {
                *reinterpret_cast<int32_t*>(targetValue) = static_cast<int32_t>(*raw);
            }
            if (strLenOrInd) {
                *strLenOrInd = sizeof(int32_t);
            }
            return SQL_SUCCESS;
        }
        case SQL_C_SBIGINT:
        {
            const auto raw = GetAsInt64(parser, ydbType);
            if (!raw) {
                return SQL_ERROR;
            }
            if (targetValue) {
                *reinterpret_cast<SQLBIGINT*>(targetValue) = static_cast<SQLBIGINT>(*raw);
            }
            if (strLenOrInd) {
                *strLenOrInd = sizeof(SQLBIGINT);
            }
            return SQL_SUCCESS;
        }
        case SQL_C_DOUBLE:
        {
            double v = 0.0;
            switch (ydbType) {
                case EPrimitiveType::Double: v = parser.GetDouble(); break;
                case EPrimitiveType::Float: v = parser.GetFloat(); break;
                default: return SQL_ERROR;
            }
            if (targetValue) {
                *reinterpret_cast<double*>(targetValue) = v;
            }
            if (strLenOrInd) {
                *strLenOrInd = sizeof(double);
            }
            return SQL_SUCCESS;
        }
        case SQL_C_CHAR:
        {
            std::string str;
            switch (ydbType) {
                case EPrimitiveType::Utf8: str = parser.GetUtf8(); break;
                case EPrimitiveType::String: str = parser.GetString(); break;
                case EPrimitiveType::Json: str = parser.GetJson(); break;
                case EPrimitiveType::JsonDocument: str = parser.GetJsonDocument(); break;
                case EPrimitiveType::Date: {
                    const TString t = parser.GetDate().FormatGmTime("%Y-%m-%d");
                    str.assign(t.data(), t.size());
                    break;
                }
                case EPrimitiveType::Date32: {
                    const auto days = parser.GetDate32().time_since_epoch();
                    if (days.count() < 0) {
                        return SQL_ERROR;
                    }
                    const TString t =
                        TInstant::Days(static_cast<ui64>(days.count())).FormatGmTime("%Y-%m-%d");
                    str.assign(t.data(), t.size());
                    break;
                }
                case EPrimitiveType::Datetime: {
                    const TString t = parser.GetDatetime().FormatGmTime("%Y-%m-%d %H:%M:%S");
                    str.assign(t.data(), t.size());
                    break;
                }
                case EPrimitiveType::Datetime64: {
                    const auto secs = parser.GetDatetime64().time_since_epoch();
                    if (secs.count() < 0) {
                        return SQL_ERROR;
                    }
                    const TString t = TInstant::Seconds(static_cast<ui64>(static_cast<std::uint64_t>(secs.count())))
                                          .FormatGmTime("%Y-%m-%d %H:%M:%S");
                    str.assign(t.data(), t.size());
                    break;
                }
                case EPrimitiveType::Timestamp: {
                    const TString t = parser.GetTimestamp().FormatGmTime("%Y-%m-%d %H:%M:%S");
                    str.assign(t.data(), t.size());
                    break;
                }
                case EPrimitiveType::Timestamp64: {
                    const auto micros = parser.GetTimestamp64().time_since_epoch();
                    if (micros.count() < 0) {
                        return SQL_ERROR;
                    }
                    const TString t =
                        TInstant::MicroSeconds(static_cast<ui64>(static_cast<std::uint64_t>(micros.count())))
                            .FormatGmTime("%Y-%m-%d %H:%M:%S");
                    str.assign(t.data(), t.size());
                    break;
                }
                case EPrimitiveType::TzDate: str = parser.GetTzDate(); break;
                case EPrimitiveType::TzDatetime: str = parser.GetTzDatetime(); break;
                case EPrimitiveType::TzTimestamp: str = parser.GetTzTimestamp(); break;
                default: return SQL_ERROR;
            }
            SQLLEN len = str.size();
            if (targetValue && bufferLength > 0) {
                SQLLEN copyLen = std::min(len, bufferLength - 1);
                memcpy(targetValue, str.data(), copyLen);
                reinterpret_cast<char*>(targetValue)[copyLen] = 0;
            }
            if (strLenOrInd) {
                *strLenOrInd = len;
            }
            if (targetValue && bufferLength > 0 && len >= static_cast<SQLLEN>(bufferLength)) {
                return SQL_SUCCESS_WITH_INFO;
            }
            return SQL_SUCCESS;
        }
        case SQL_C_BIT:
        {
            const auto raw = GetAsInt64(parser, ydbType);
            if (!raw) {
                return SQL_ERROR;
            }
            if (*raw != 0 && *raw != 1) {
                SetNumericOutOfRange();
                return SQL_ERROR;
            }
            const char v = *raw != 0 ? 1 : 0;
            if (targetValue) {
                *reinterpret_cast<char*>(targetValue) = v;
            }
            if (strLenOrInd) {
                *strLenOrInd = sizeof(char);
            }
            return SQL_SUCCESS;
        }
        default:
            return SQL_ERROR;
    }
}

const char* ConsumeLastConvertSqlState() {
    const char* result = LastConvertSqlState;
    LastConvertSqlState = nullptr;
    return result;
}

} // namespace NOdbc
} // namespace NYdb
