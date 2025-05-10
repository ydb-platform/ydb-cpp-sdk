#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TYdbValueImpl TYdbValue;
typedef struct TYdbParamsImpl TYdbParams;

typedef enum {
    YDB_VALUE_OK,
    YDB_VALUE_ERROR,
} EYdbValueStatus;

typedef enum {
    YDB_TYPE_KIND_UNDEFINED,
    YDB_TYPE_KIND_PRIMITIVE,
    YDB_TYPE_KIND_OPTIONAL,
    YDB_TYPE_KIND_LIST,
    YDB_TYPE_KIND_TUPLE,
    YDB_TYPE_KIND_STRUCT,
    YDB_TYPE_KIND_DICT,
    YDB_TYPE_KIND_VARIANT,
} EYdbTypeKind;

typedef enum {
    YDB_PRIMITIVE_TYPE_UNDEFINED,
    YDB_PRIMITIVE_TYPE_BOOL,
    YDB_PRIMITIVE_TYPE_INT8,
    YDB_PRIMITIVE_TYPE_UINT8,
    YDB_PRIMITIVE_TYPE_INT16,
    YDB_PRIMITIVE_TYPE_UINT16,
    YDB_PRIMITIVE_TYPE_INT32,
    YDB_PRIMITIVE_TYPE_UINT32,
    YDB_PRIMITIVE_TYPE_INT64,
    YDB_PRIMITIVE_TYPE_UINT64,
    YDB_PRIMITIVE_TYPE_FLOAT,
    YDB_PRIMITIVE_TYPE_DOUBLE,
    YDB_PRIMITIVE_TYPE_STRING,
    YDB_PRIMITIVE_TYPE_UTF8,
    YDB_PRIMITIVE_TYPE_YSON,
    YDB_PRIMITIVE_TYPE_JSON,
    YDB_PRIMITIVE_TYPE_JSON_DOCUMENT,
    YDB_PRIMITIVE_TYPE_DYNUMBER,
} EYdbPrimitiveType;

EYdbTypeKind YdbGetTypeKind(TYdbValue* value);
EYdbPrimitiveType YdbGetPrimitiveType(TYdbValue* value);

EYdbValueStatus YdbGetBool(TYdbValue* value, bool* result);
EYdbValueStatus YdbGetInt8(TYdbValue* value, int8_t* result);
EYdbValueStatus YdbGetUint8(TYdbValue* value, uint8_t* result);
EYdbValueStatus YdbGetInt16(TYdbValue* value, int16_t* result);
EYdbValueStatus YdbGetUint16(TYdbValue* value, uint16_t* result);
EYdbValueStatus YdbGetInt32(TYdbValue* value, int32_t* result);
EYdbValueStatus YdbGetUint32(TYdbValue* value, uint32_t* result);
EYdbValueStatus YdbGetInt64(TYdbValue* value, int64_t* result);
EYdbValueStatus YdbGetUint64(TYdbValue* value, uint64_t* result);
EYdbValueStatus YdbGetFloat(TYdbValue* value, float* result);
EYdbValueStatus YdbGetDouble(TYdbValue* value, double* result);
EYdbValueStatus YdbGetString(TYdbValue* value, char** result);
EYdbValueStatus YdbGetUtf8(TYdbValue* value, char** result);
EYdbValueStatus YdbGetYson(TYdbValue* value, char** result);
EYdbValueStatus YdbGetJson(TYdbValue* value, char** result);
EYdbValueStatus YdbGetJsonDocument(TYdbValue* value, char** result);
EYdbValueStatus YdbGetDyNumber(TYdbValue* value, char** result);

void YdbDestroyValue(TYdbValue* value);

#ifdef __cplusplus
}
#endif
