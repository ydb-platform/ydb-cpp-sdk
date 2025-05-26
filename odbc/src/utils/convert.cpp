#include "convert.h"

#include <util/generic/singleton.h>

namespace NYdb {
namespace NOdbc {

template<SQLSMALLINT CType>
struct TSqlTypeTraits;

template<> struct TSqlTypeTraits<SQL_C_CHAR> { using Type = std::string; };
template<> struct TSqlTypeTraits<SQL_C_BINARY> { using Type = std::string; };
template<> struct TSqlTypeTraits<SQL_C_SBIGINT> { using Type = SQLBIGINT; };
template<> struct TSqlTypeTraits<SQL_C_UBIGINT> { using Type = SQLUBIGINT; };
template<> struct TSqlTypeTraits<SQL_C_LONG> { using Type = SQLINTEGER; };
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
    Data = std::string(static_cast<const char*>(param.ParameterValuePtr), param.BufferLength);
}

template<>
TTypedValue<SQL_C_BINARY>::TTypedValue(const TBoundParam& param) {
    Data = std::string(static_cast<const char*>(param.ParameterValuePtr), param.BufferLength);
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

void ConvertValue(const TBoundParam& param, TParamValueBuilder& builder) {
    auto converter = TConverterRegistry::GetInstance().GetConverter(param.ValueType, param.ParameterType);
    if (converter) {
        converter->AddToBuilder(param, builder);
    } else {
        throw 1; // TODO: throw exception
    }
}

} // namespace NYdb
} // namespace NOdbc
