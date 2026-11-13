#pragma once

#include "statement.h"

namespace NYdb::NOdbc::NMetadata {

SQLRETURN GetInfo(TConnection* connection, SQLUSMALLINT infoType, SQLPOINTER infoValuePtr,
                  SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthPtr);

SQLRETURN GetFunctions(SQLUSMALLINT functionId, SQLUSMALLINT* supportedPtr);

SQLRETURN DescribeCol(TStatement* statement, SQLUSMALLINT columnNumber, SQLCHAR* columnName,
                      SQLSMALLINT bufferLength, SQLSMALLINT* nameLengthPtr,
                      SQLSMALLINT* dataTypePtr, SQLULEN* columnSizePtr,
                      SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr);

SQLRETURN ColAttribute(TStatement* statement, SQLUSMALLINT columnNumber,
                       SQLUSMALLINT fieldIdentifier, SQLPOINTER characterAttributePtr,
                       SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthAttributePtr,
                       SQLLEN* numericAttributePtr);

} // namespace NYdb::NOdbc::NMetadata
