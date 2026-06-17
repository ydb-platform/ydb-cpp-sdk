#pragma once

#include "connection.h"
#include "statement.h"

namespace NYdb::NOdbc {
namespace NMetadata {

SQLRETURN GetInfo(
    TConnection* conn,
    SQLUSMALLINT infoType,
    SQLPOINTER infoValuePtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthPtr);

SQLRETURN GetFunctions(
    SQLUSMALLINT functionId,
    SQLUSMALLINT* supportedPtr);

SQLRETURN DescribeCol(
    TStatement* stmt,
    SQLUSMALLINT columnNumber,
    SQLCHAR* columnName,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* nameLengthPtr,
    SQLSMALLINT* dataTypePtr,
    SQLULEN* columnSizePtr,
    SQLSMALLINT* decimalDigitsPtr,
    SQLSMALLINT* nullablePtr);

SQLRETURN ColAttribute(
    TStatement* stmt,
    SQLUSMALLINT columnNumber,
    SQLUSMALLINT fieldIdentifier,
    SQLPOINTER characterAttributePtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthAttributePtr,
    SQLLEN* numericAttributePtr);

} // namespace NMetadata
} // namespace NYdb::NOdbc
