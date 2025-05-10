#pragma once

#include "value.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TYdbResultSetImpl TYdbResultSet;

typedef enum {
    YDB_RESULT_SET_OK,
    YDB_RESULT_SET_ERROR,
} EYdbResultSetStatus;

int YdbGetColumnsCount(TYdbResultSet* resultSet);
int YdbGetRowsCount(TYdbResultSet* resultSet);
int YdbIsTruncated(TYdbResultSet* resultSet);

const char* YdbGetColumnName(TYdbResultSet* resultSet, size_t index);
int YdbGetColumnIndex(TYdbResultSet* resultSet, const char* name);

TYdbValue* YdbGetValue(TYdbResultSet* resultSet, size_t rowIndex, const char* name);
TYdbValue* YdbGetValueByIndex(TYdbResultSet* resultSet, size_t rowIndex, size_t columnIndex);

void YdbDestroyResultSet(TYdbResultSet* resultSet);

#ifdef __cplusplus
}
#endif
