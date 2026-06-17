#include "metadata.h"

#include "utils/diag.h"

#include <algorithm>
#include <cstring>

namespace NYdb::NOdbc {
namespace {

SQLRETURN WriteInfoString(
    TConnection* conn,
    const char* value,
    SQLPOINTER infoValuePtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthPtr) {
    return Diag::WriteOdbcString(*conn, value, infoValuePtr, bufferLength, stringLengthPtr);
}

template <typename T>
SQLRETURN WriteInfoScalar(
    TConnection* conn,
    T value,
    SQLPOINTER infoValuePtr,
    SQLSMALLINT* stringLengthPtr) {
    if (!infoValuePtr) {
        return conn->AddError("HY009", 0, "Invalid use of null pointer");
    }
    *reinterpret_cast<T*>(infoValuePtr) = value;
    if (stringLengthPtr) {
        *stringLengthPtr = static_cast<SQLSMALLINT>(sizeof(T));
    }
    return SQL_SUCCESS;
}


bool IsSupportedFunction(SQLUSMALLINT functionId) {
    switch (functionId) {
        case SQL_API_SQLALLOCHANDLE:
        case SQL_API_SQLBINDCOL:
        case SQL_API_SQLBINDPARAMETER:
        case SQL_API_SQLCANCEL:
        case SQL_API_SQLCLOSECURSOR:
        case SQL_API_SQLCOLATTRIBUTE:
        case SQL_API_SQLCOLUMNS:
        case SQL_API_SQLCONNECT:
        case SQL_API_SQLCOPYDESC:
        case SQL_API_SQLDESCRIBECOL:
        case SQL_API_SQLDESCRIBEPARAM:
        case SQL_API_SQLDISCONNECT:
        case SQL_API_SQLDRIVERCONNECT:
        case SQL_API_SQLENDTRAN:
        case SQL_API_SQLEXECDIRECT:
        case SQL_API_SQLEXECUTE:
        case SQL_API_SQLFETCH:
        case SQL_API_SQLFETCHSCROLL:
        case SQL_API_SQLFOREIGNKEYS:
        case SQL_API_SQLFREEHANDLE:
        case SQL_API_SQLFREESTMT:
        case SQL_API_SQLGETCURSORNAME:
        case SQL_API_SQLGETDATA:
        case SQL_API_SQLGETDESCFIELD:
        case SQL_API_SQLGETDESCREC:
        case SQL_API_SQLGETDIAGFIELD:
        case SQL_API_SQLGETDIAGREC:
        case SQL_API_SQLGETFUNCTIONS:
        case SQL_API_SQLGETCONNECTATTR:
        case SQL_API_SQLGETENVATTR:
        case SQL_API_SQLGETINFO:
        case SQL_API_SQLGETSTMTATTR:
        case SQL_API_SQLGETTYPEINFO:
        case SQL_API_SQLMORERESULTS:
        case SQL_API_SQLNATIVESQL:
        case SQL_API_SQLNUMPARAMS:
        case SQL_API_SQLNUMRESULTCOLS:
        case SQL_API_SQLPARAMDATA:
        case SQL_API_SQLPREPARE:
        case SQL_API_SQLPRIMARYKEYS:
        case SQL_API_SQLPUTDATA:
        case SQL_API_SQLROWCOUNT:
        case SQL_API_SQLSETCONNECTATTR:
        case SQL_API_SQLSETCURSORNAME:
        case SQL_API_SQLSETDESCFIELD:
        case SQL_API_SQLSETDESCREC:
        case SQL_API_SQLSETENVATTR:
        case SQL_API_SQLSETSTMTATTR:
        case SQL_API_SQLSPECIALCOLUMNS:
        case SQL_API_SQLSTATISTICS:
        case SQL_API_SQLTABLES:
            return true;
        default:
            return false;
    }
}

} // namespace

SQLRETURN NMetadata::GetInfo(
    TConnection* conn,
    SQLUSMALLINT infoType,
    SQLPOINTER infoValuePtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthPtr) {
    switch (infoType) {
        // Driver Information
        case SQL_DRIVER_NAME:
            return WriteInfoString(conn, "ydb-odbc", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DRIVER_VER:
            return WriteInfoString(conn, "unknown", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DRIVER_ODBC_VER:
            return WriteInfoString(conn, "03.00", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_ODBC_INTERFACE_CONFORMANCE:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_OIC_CORE, infoValuePtr, stringLengthPtr);
        case SQL_ODBC_API_CONFORMANCE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_OAC_LEVEL1, infoValuePtr, stringLengthPtr);
        case SQL_ODBC_SAG_CLI_CONFORMANCE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_OSCC_NOT_COMPLIANT, infoValuePtr, stringLengthPtr);
        case SQL_ODBC_SQL_CONFORMANCE:
            return WriteInfoScalar<SQLSMALLINT>(conn, SQL_OSC_MINIMUM, infoValuePtr, stringLengthPtr);
        case SQL_MAX_TABLE_NAME_LEN:
        case SQL_MAX_COLUMN_NAME_LEN:
        case SQL_MAX_CATALOG_NAME_LEN:
        case SQL_MAX_IDENTIFIER_LEN:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 255, infoValuePtr, stringLengthPtr);
        case SQL_MAX_SCHEMA_NAME_LEN:
        case SQL_MAX_PROCEDURE_NAME_LEN:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 0, infoValuePtr, stringLengthPtr);
        case SQL_MAX_USER_NAME_LEN:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 128, infoValuePtr, stringLengthPtr);
        case SQL_MAX_DRIVER_CONNECTIONS:
        case SQL_MAX_CONCURRENT_ACTIVITIES:
        case SQL_MAX_STATEMENT_LEN:
        case SQL_MAX_BINARY_LITERAL_LEN:
        case SQL_MAX_CHAR_LITERAL_LEN:
        case SQL_MAX_COLUMNS_IN_GROUP_BY:
        case SQL_MAX_COLUMNS_IN_ORDER_BY:
        case SQL_MAX_COLUMNS_IN_INDEX:
        case SQL_MAX_COLUMNS_IN_SELECT:
        case SQL_MAX_COLUMNS_IN_TABLE:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);
        case SQL_SEARCH_PATTERN_ESCAPE:
            return WriteInfoString(conn, "\\", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_KEYWORDS:
        case SQL_SPECIAL_CHARACTERS:
            return WriteInfoString(conn, "", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_CONCAT_NULL_BEHAVIOR:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_CB_NULL, infoValuePtr, stringLengthPtr);
        case SQL_NULL_COLLATION:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_NC_HIGH, infoValuePtr, stringLengthPtr);
        case SQL_MAX_CURSOR_NAME_LEN:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 128, infoValuePtr, stringLengthPtr);

        // DBMS Information
        case SQL_DBMS_NAME:
            return WriteInfoString(conn, "YDB", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DBMS_VER:
            return WriteInfoString(conn, conn->GetDbmsVersion().c_str(), infoValuePtr, bufferLength, stringLengthPtr);

        // Identifier Handling
        case SQL_IDENTIFIER_QUOTE_CHAR:
            return WriteInfoString(conn, "\"", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_IDENTIFIER_CASE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_IC_LOWER, infoValuePtr, stringLengthPtr);

        // Catalog Support
        case SQL_CATALOG_NAME:
            return WriteInfoString(conn, "Y", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_CATALOG_NAME_SEPARATOR:
            return WriteInfoString(conn, "/", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_CATALOG_TERM:
            return WriteInfoString(conn, "path", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_CATALOG_USAGE:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_CU_DML_STATEMENTS, infoValuePtr, stringLengthPtr);

        // Schema Support (YDB doesn't use schemas)
        case SQL_SCHEMA_USAGE:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);
        case SQL_SCHEMA_TERM:
            return WriteInfoString(conn, "", infoValuePtr, bufferLength, stringLengthPtr);

        // Data Source Capabilities
        case SQL_DATA_SOURCE_READ_ONLY:
            return WriteInfoString(
                conn, conn->IsDataSourceReadOnly() ? "Y" : "N", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DATA_SOURCE_NAME:
            return WriteInfoString(conn, conn->GetDataSourceName().c_str(), infoValuePtr, bufferLength, stringLengthPtr);

        // Result Set Capabilities
        case SQL_MULT_RESULT_SETS:
            return WriteInfoString(conn, "N", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DYNAMIC_CURSOR_ATTRIBUTES1:
        case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1:
        case SQL_STATIC_CURSOR_ATTRIBUTES1:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_CA1_NEXT, infoValuePtr, stringLengthPtr);
        case SQL_CURSOR_COMMIT_BEHAVIOR:
        case SQL_CURSOR_ROLLBACK_BEHAVIOR:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_CB_CLOSE, infoValuePtr, stringLengthPtr);

        // Transaction Support
        case SQL_TXN_CAPABLE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_TC_ALL, infoValuePtr, stringLengthPtr);
        case SQL_DEFAULT_TXN_ISOLATION:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_TXN_SERIALIZABLE, infoValuePtr, stringLengthPtr);
        case SQL_TXN_ISOLATION_OPTION:
            return WriteInfoScalar<SQLUINTEGER>(
                conn, conn->GetSupportedTxnIsolationOptions(), infoValuePtr, stringLengthPtr);

        // Stored Procedures (not supported)
        case SQL_PROCEDURES:
            return WriteInfoString(conn, "N", infoValuePtr, bufferLength, stringLengthPtr);

        case SQL_OUTER_JOINS:
            return WriteInfoString(conn, "Y", infoValuePtr, bufferLength, stringLengthPtr);

        // Positioned Operations (not supported)
        case SQL_POSITIONED_STATEMENTS:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);

        // Batch Operations (not supported)
        case SQL_BATCH_SUPPORT:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);
        case SQL_BATCH_ROW_COUNT:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);

        // Bookmarks (not supported)
        case SQL_BOOKMARK_PERSISTENCE:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);

        // Named Cursors (not supported)
        case SQL_FILE_USAGE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_FILE_NOT_SUPPORTED, infoValuePtr, stringLengthPtr);

        // GetData Extensions
        case SQL_GETDATA_EXTENSIONS:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER, infoValuePtr, stringLengthPtr);

        // Async Execution (not supported)
        case SQL_ASYNC_MODE:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_AM_NONE, infoValuePtr, stringLengthPtr);

        case SQL_QUOTED_IDENTIFIER_CASE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_IC_SENSITIVE, infoValuePtr, stringLengthPtr);

        default:
            return conn->AddError("HYC00", 0, "Optional feature not implemented");
    }
}


SQLRETURN NMetadata::GetFunctions(SQLUSMALLINT functionId, SQLUSMALLINT* supportedPtr) {
    if (!supportedPtr) {
        return SQL_ERROR;
    }

    if (functionId == SQL_API_ALL_FUNCTIONS) {
        std::memset(supportedPtr, 0, 100 * sizeof(SQLUSMALLINT));
        for (SQLUSMALLINT id = 0; id < 100; ++id) {
            if (IsSupportedFunction(id)) {
                supportedPtr[id] = SQL_TRUE;
            }
        }
        return SQL_SUCCESS;
    }

    if (functionId == SQL_API_ODBC3_ALL_FUNCTIONS) {
        std::memset(supportedPtr, 0, SQL_API_ODBC3_ALL_FUNCTIONS_SIZE * sizeof(SQLUSMALLINT));
        for (SQLUSMALLINT id = 0; id < SQL_API_ODBC3_ALL_FUNCTIONS_SIZE * 16; ++id) {
            if (IsSupportedFunction(id)) {
                supportedPtr[id >> 4] |= (1 << (id & 0x000F));
            }
        }
        return SQL_SUCCESS;
    }

    *supportedPtr = IsSupportedFunction(functionId) ? SQL_TRUE : SQL_FALSE;
    return SQL_SUCCESS;
}

SQLRETURN NMetadata::DescribeCol(
    TStatement* stmt,
    SQLUSMALLINT columnNumber,
    SQLCHAR* columnName,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* nameLengthPtr,
    SQLSMALLINT* dataTypePtr,
    SQLULEN* columnSizePtr,
    SQLSMALLINT* decimalDigitsPtr,
    SQLSMALLINT* nullablePtr) {
    const auto& columns = stmt->GetColumnMeta();
    if (columnNumber < 1 || columnNumber > columns.size()) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }

    const auto& column = columns[columnNumber - 1];
    const SQLRETURN nameRc = Diag::WriteOdbcString(*stmt, column.Name, columnName, bufferLength, nameLengthPtr);
    if (nameRc != SQL_SUCCESS) {
        return nameRc;
    }
    if (dataTypePtr) {
        *dataTypePtr = column.SqlType;
    }
    if (columnSizePtr) {
        *columnSizePtr = column.Size;
    }
    if (decimalDigitsPtr) {
        *decimalDigitsPtr = column.DecimalDigits;
    }
    if (nullablePtr) {
        *nullablePtr = column.Nullable;
    }
    return SQL_SUCCESS;
}

SQLRETURN NMetadata::ColAttribute(
    TStatement* stmt,
    SQLUSMALLINT columnNumber,
    SQLUSMALLINT fieldIdentifier,
    SQLPOINTER characterAttributePtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthAttributePtr,
    SQLLEN* numericAttributePtr) {
    SQLCHAR name[256] = {};
    SQLSMALLINT nameLength = 0;
    SQLSMALLINT dataType = 0;
    SQLULEN columnSize = 0;
    SQLSMALLINT decimalDigits = 0;
    SQLSMALLINT nullable = 0;

    const SQLRETURN describeRc = DescribeCol(
        stmt, columnNumber, name, sizeof(name), &nameLength, &dataType, &columnSize, &decimalDigits, &nullable);
    if (describeRc != SQL_SUCCESS) {
        return describeRc;
    }

    const auto setNumericAttr = [&](SQLLEN value) -> SQLRETURN {
        if (!numericAttributePtr) {
            return stmt->AddError("HY009", 0, "Invalid use of null pointer");
        }
        *numericAttributePtr = value;
        return SQL_SUCCESS;
    };

    switch (fieldIdentifier) {
        case SQL_DESC_NAME:
        case SQL_COLUMN_NAME: {
            if (!characterAttributePtr && bufferLength != 0) {
                return stmt->AddError("HY090", 0, "Invalid string or buffer length");
            }
            const SQLSMALLINT fullLen = nameLength;
            if (stringLengthAttributePtr) {
                *stringLengthAttributePtr = fullLen;
            }
            if (bufferLength == 0) {
                return fullLen == 0 ? SQL_SUCCESS
                    : stmt->AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
            }
            auto* out = reinterpret_cast<char*>(characterAttributePtr);
            const SQLSMALLINT copyLen = static_cast<SQLSMALLINT>(std::min<int>(fullLen, bufferLength - 1));
            if (copyLen > 0) {
                std::memcpy(out, name, static_cast<size_t>(copyLen));
            }
            if (out) {
                out[copyLen] = '\0';
            }
            if (copyLen < fullLen) {
                return stmt->AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
            }
            return SQL_SUCCESS;
        }
        case SQL_DESC_TYPE:
        case SQL_COLUMN_TYPE:
            return setNumericAttr(dataType);
        case SQL_DESC_LENGTH:
        case SQL_COLUMN_LENGTH:
            return setNumericAttr(static_cast<SQLLEN>(columnSize));
        case SQL_DESC_PRECISION:
        case SQL_COLUMN_PRECISION:
            return setNumericAttr(static_cast<SQLLEN>(columnSize));
        case SQL_DESC_SCALE:
        case SQL_COLUMN_SCALE:
            return setNumericAttr(decimalDigits);
        case SQL_DESC_NULLABLE:
        case SQL_COLUMN_NULLABLE:
            return setNumericAttr(nullable);
        default:
            return stmt->AddError("HYC00", 0, "Optional feature not implemented");
    }
}

} // namespace NYdb::NOdbc
