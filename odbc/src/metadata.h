#pragma once

#include "connection.h"
#include "statement.h"

namespace NYdb::NOdbc {

class TMetadata {
public:
    static SQLRETURN GetInfo(
        TConnection* conn,
        SQLUSMALLINT infoType,
        SQLPOINTER infoValuePtr,
        SQLSMALLINT bufferLength,
        SQLSMALLINT* stringLengthPtr);

    static SQLRETURN GetFunctions(
        SQLUSMALLINT functionId,
        SQLUSMALLINT* supportedPtr);

    static SQLRETURN DescribeCol(
        TStatement* stmt,
        SQLUSMALLINT columnNumber,
        SQLCHAR* columnName,
        SQLSMALLINT bufferLength,
        SQLSMALLINT* nameLengthPtr,
        SQLSMALLINT* dataTypePtr,
        SQLULEN* columnSizePtr,
        SQLSMALLINT* decimalDigitsPtr,
        SQLSMALLINT* nullablePtr);
};

} // namespace NYdb::NOdbc
