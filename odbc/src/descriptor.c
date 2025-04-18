#include "ydb_odbc.h"
#include <string.h>
#include <stdlib.h>

// Структура для хранения поля дескриптора
typedef struct {
    SQLSMALLINT field_identifier;
    char value[1024];
} YDB_DESCRIPTOR_FIELD;

// Структура для хранения записи дескриптора
typedef struct {
    YDB_DESCRIPTOR_FIELD* fields;
    size_t fields_count;
} YDB_DESCRIPTOR_RECORD;

SQLRETURN YDB_SQLGetDescField(SQLHDESC DescriptorHandle, SQLSMALLINT RecNumber,
                            SQLSMALLINT FieldIdentifier, SQLPOINTER Value,
                            SQLINTEGER BufferLength, SQLINTEGER* StringLength) {
    YDB_DESCRIPTOR* desc = (YDB_DESCRIPTOR*)DescriptorHandle;
    if (!desc || !desc->descriptors || RecNumber < 1 || RecNumber > desc->descriptors_size) {
        return SQL_ERROR;
    }

    YDB_DESCRIPTOR_RECORD* record = (YDB_DESCRIPTOR_RECORD*)desc->descriptors[RecNumber - 1];
    if (!record) {
        return SQL_ERROR;
    }

    for (size_t i = 0; i < record->fields_count; i++) {
        if (record->fields[i].field_identifier == FieldIdentifier) {
            if (Value && BufferLength > 0) {
                strncpy((char*)Value, record->fields[i].value, BufferLength - 1);
                if (StringLength) {
                    *StringLength = strlen(record->fields[i].value);
                }
                return SQL_SUCCESS;
            }
            break;
        }
    }

    return SQL_ERROR;
}

SQLRETURN YDB_SQLSetDescField(SQLHDESC DescriptorHandle, SQLSMALLINT RecNumber,
                            SQLSMALLINT FieldIdentifier, SQLPOINTER Value,
                            SQLINTEGER BufferLength) {
    YDB_DESCRIPTOR* desc = (YDB_DESCRIPTOR*)DescriptorHandle;
    if (!desc || RecNumber < 1) {
        return SQL_ERROR;
    }

    // Увеличиваем размер массива дескрипторов, если нужно
    if (RecNumber > desc->descriptors_size) {
        void** new_descriptors = realloc(desc->descriptors, RecNumber * sizeof(void*));
        if (!new_descriptors) {
            return SQL_ERROR;
        }

        // Инициализируем новые записи
        for (size_t i = desc->descriptors_size; i < RecNumber; i++) {
            YDB_DESCRIPTOR_RECORD* record = calloc(1, sizeof(YDB_DESCRIPTOR_RECORD));
            if (!record) {
                // Освобождаем память в случае ошибки
                for (size_t j = desc->descriptors_size; j < i; j++) {
                    free(new_descriptors[j]);
                }
                free(new_descriptors);
                return SQL_ERROR;
            }
            new_descriptors[i] = record;
        }

        desc->descriptors = new_descriptors;
        desc->descriptors_size = RecNumber;
    }

    YDB_DESCRIPTOR_RECORD* record = (YDB_DESCRIPTOR_RECORD*)desc->descriptors[RecNumber - 1];
    if (!record) {
        record = calloc(1, sizeof(YDB_DESCRIPTOR_RECORD));
        if (!record) {
            return SQL_ERROR;
        }
        desc->descriptors[RecNumber - 1] = record;
    }

    // Проверяем, существует ли уже поле с таким идентификатором
    for (size_t i = 0; i < record->fields_count; i++) {
        if (record->fields[i].field_identifier == FieldIdentifier) {
            // Обновляем значение
            strncpy(record->fields[i].value, (char*)Value, sizeof(record->fields[i].value) - 1);
            return SQL_SUCCESS;
        }
    }

    // Добавляем новое поле
    YDB_DESCRIPTOR_FIELD* new_fields = realloc(record->fields, (record->fields_count + 1) * sizeof(YDB_DESCRIPTOR_FIELD));
    if (!new_fields) {
        return SQL_ERROR;
    }

    record->fields = new_fields;
    record->fields[record->fields_count].field_identifier = FieldIdentifier;
    strncpy(record->fields[record->fields_count].value, (char*)Value, sizeof(record->fields[record->fields_count].value) - 1);
    record->fields_count++;

    return SQL_SUCCESS;
} 