#include <ydb-cpp-sdk/c_api/value.h>

#include <ydb-cpp-sdk/client/value/value.h>

#include "impl/value_impl.h" // NOLINT

extern "C" {

EYdbTypeKind YdbGetTypeKind(TYdbValue* value) {
    if (!value || !value->value.has_value()) {
        return YDB_TYPE_KIND_UNDEFINED;
    }

    NYdb::TValueParser valueParser(*value->value);

    switch (valueParser.GetKind()) {
        case NYdb::TTypeParser::ETypeKind::Primitive:
            return YDB_TYPE_KIND_PRIMITIVE;
        case NYdb::TTypeParser::ETypeKind::Optional:
            return YDB_TYPE_KIND_OPTIONAL;
        case NYdb::TTypeParser::ETypeKind::List:
            return YDB_TYPE_KIND_LIST;
        case NYdb::TTypeParser::ETypeKind::Struct:
            return YDB_TYPE_KIND_STRUCT;
        case NYdb::TTypeParser::ETypeKind::Tuple:
            return YDB_TYPE_KIND_TUPLE;
        case NYdb::TTypeParser::ETypeKind::Dict:
            return YDB_TYPE_KIND_DICT;
        case NYdb::TTypeParser::ETypeKind::Variant:
            return YDB_TYPE_KIND_VARIANT;
        default:
            return YDB_TYPE_KIND_UNDEFINED;
    }
}

EYdbPrimitiveType YdbGetPrimitiveType(TYdbValue* value) {
    if (!value || !value->value.has_value()) {
        return YDB_PRIMITIVE_TYPE_UNDEFINED;
    }

    try {
        NYdb::TValueParser valueParser(*value->value);

        switch (valueParser.GetPrimitiveType()) {
            case NYdb::EPrimitiveType::Int8:
                return YDB_PRIMITIVE_TYPE_INT8;
            case NYdb::EPrimitiveType::Uint8:
                return YDB_PRIMITIVE_TYPE_UINT8;
            case NYdb::EPrimitiveType::Int16:
                return YDB_PRIMITIVE_TYPE_INT16;
            case NYdb::EPrimitiveType::Uint16:
                return YDB_PRIMITIVE_TYPE_UINT16;
            case NYdb::EPrimitiveType::Int32:
                return YDB_PRIMITIVE_TYPE_INT32;
            case NYdb::EPrimitiveType::Uint32:
                return YDB_PRIMITIVE_TYPE_UINT32;
            case NYdb::EPrimitiveType::Int64:
                return YDB_PRIMITIVE_TYPE_INT64;
            case NYdb::EPrimitiveType::Uint64:
                return YDB_PRIMITIVE_TYPE_UINT64;
            case NYdb::EPrimitiveType::Float:
                return YDB_PRIMITIVE_TYPE_FLOAT;
            case NYdb::EPrimitiveType::Double:
                return YDB_PRIMITIVE_TYPE_DOUBLE;
            case NYdb::EPrimitiveType::String:
                return YDB_PRIMITIVE_TYPE_STRING;
            case NYdb::EPrimitiveType::Utf8:
                return YDB_PRIMITIVE_TYPE_UTF8;
            case NYdb::EPrimitiveType::Yson:
                return YDB_PRIMITIVE_TYPE_YSON;
            case NYdb::EPrimitiveType::Json:
                return YDB_PRIMITIVE_TYPE_JSON;
            case NYdb::EPrimitiveType::JsonDocument:
                return YDB_PRIMITIVE_TYPE_JSON_DOCUMENT;
            case NYdb::EPrimitiveType::DyNumber:
                return YDB_PRIMITIVE_TYPE_DYNUMBER;
            default:
                return YDB_PRIMITIVE_TYPE_UNDEFINED;
        }
    } catch (...) {
        return YDB_PRIMITIVE_TYPE_UNDEFINED;
    }
}

EYdbValueStatus YdbGetInt8(TYdbValue* value, int8_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetInt8();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetUint8(TYdbValue* value, uint8_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetUint8();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetInt16(TYdbValue* value, int16_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetInt16();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetUint16(TYdbValue* value, uint16_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetUint16();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetInt32(TYdbValue* value, int32_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetInt32();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetUint32(TYdbValue* value, uint32_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetUint32();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetInt64(TYdbValue* value, int64_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetInt64();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetUint64(TYdbValue* value, uint64_t* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetUint64();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetFloat(TYdbValue* value, float* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetFloat();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetDouble(TYdbValue* value, double* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetDouble();
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetString(TYdbValue* value, char** result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = strndup(valueParser.GetString().c_str(), valueParser.GetString().size());
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetUtf8(TYdbValue* value, char** result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = strndup(valueParser.GetUtf8().c_str(), valueParser.GetUtf8().size());
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetYson(TYdbValue* value, char** result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = strndup(valueParser.GetYson().c_str(), valueParser.GetYson().size());
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetJson(TYdbValue* value, char** result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = strndup(valueParser.GetJson().c_str(), valueParser.GetJson().size());
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetJsonDocument(TYdbValue* value, char** result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = strndup(valueParser.GetJsonDocument().c_str(), valueParser.GetJsonDocument().size());
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetDyNumber(TYdbValue* value, char** result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = strndup(valueParser.GetDyNumber().c_str(), valueParser.GetDyNumber().size());
    return YDB_VALUE_OK;
}

EYdbValueStatus YdbGetBool(TYdbValue* value, bool* result) {
    if (!value || !value->value.has_value()) {
        return YDB_VALUE_ERROR;
    }

    NYdb::TValueParser valueParser(*value->value);

    *result = valueParser.GetBool();
    return YDB_VALUE_OK;
}

void YdbDestroyValue(TYdbValue* value) {
    delete value;
}

}
