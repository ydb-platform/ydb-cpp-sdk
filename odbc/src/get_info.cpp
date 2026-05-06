#include "get_info.h"

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
    if (!infoValuePtr) {
        return conn->AddError("HY009", 0, "Invalid use of null pointer");
    }
    if (bufferLength < 0) {
        return conn->AddError("HY090", 0, "Invalid string or buffer length");
    }
    const SQLSMALLINT fullLen = static_cast<SQLSMALLINT>(std::strlen(value));
    if (stringLengthPtr) {
        *stringLengthPtr = fullLen;
    }
    if (bufferLength == 0) {
        return fullLen == 0 ? SQL_SUCCESS : conn->AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
    }

    auto* out = reinterpret_cast<char*>(infoValuePtr);
    const SQLSMALLINT copyLen = static_cast<SQLSMALLINT>(std::min<int>(fullLen, bufferLength - 1));
    if (copyLen > 0) {
        std::memcpy(out, value, static_cast<size_t>(copyLen));
    }
    out[copyLen] = '\0';
    if (copyLen < fullLen) {
        return conn->AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
    }
    return SQL_SUCCESS;
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

} // namespace

SQLRETURN TInfoProvider::GetInfo(
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
            return WriteInfoString(conn, "03.80.0000", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DRIVER_ODBC_VER:
            return WriteInfoString(conn, "03.80", infoValuePtr, bufferLength, stringLengthPtr);

        // DBMS Information
        case SQL_DBMS_NAME:
            return WriteInfoString(conn, "YDB", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DBMS_VER:
            return WriteInfoString(conn, "3.8.0", infoValuePtr, bufferLength, stringLengthPtr);

        // Identifier Handling
        case SQL_IDENTIFIER_QUOTE_CHAR:
            return WriteInfoString(conn, "\"", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_IDENTIFIER_CASE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_IC_MIXED, infoValuePtr, stringLengthPtr);

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
            return WriteInfoString(conn, "N", infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DATA_SOURCE_NAME:
            return WriteInfoString(conn, "YDB", infoValuePtr, bufferLength, stringLengthPtr);

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
                conn,
                SQL_TXN_READ_UNCOMMITTED | SQL_TXN_READ_COMMITTED | SQL_TXN_REPEATABLE_READ | SQL_TXN_SERIALIZABLE,
                infoValuePtr,
                stringLengthPtr);

        // Connection Limits
        case SQL_MAX_CONCURRENT_ACTIVITIES:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 1, infoValuePtr, stringLengthPtr);
        case SQL_MAX_DRIVER_CONNECTIONS:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 0, infoValuePtr, stringLengthPtr);

        // SQL Support
        case SQL_SQL_CONFORMANCE:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_SC_SQL92_ENTRY, infoValuePtr, stringLengthPtr);
        case SQL_SQL92_RELATIONAL_JOIN_OPERATORS:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);
        case SQL_SUBQUERIES:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_SQ_CORRELATED_SUBQUERIES, infoValuePtr, stringLengthPtr);

        // Supported Statements
        case SQL_SQL92_PREDICATES:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_SP_IN | SQL_SP_ISNOTNULL | SQL_SP_ISNULL | SQL_SP_LIKE, infoValuePtr, stringLengthPtr);
        case SQL_SQL92_VALUE_EXPRESSIONS:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_SVE_CAST | SQL_SVE_CASE | SQL_SVE_COALESCE, infoValuePtr, stringLengthPtr);
        case SQL_AGGREGATE_FUNCTIONS:
            return WriteInfoScalar<SQLUINTEGER>(conn, SQL_AF_ALL | SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM, infoValuePtr, stringLengthPtr);

        // Data Type Limits
        case SQL_MAX_CHAR_LITERAL_LEN:
            return WriteInfoScalar<SQLUINTEGER>(conn, 0, infoValuePtr, stringLengthPtr);
        case SQL_MAX_COLUMN_NAME_LEN:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 255, infoValuePtr, stringLengthPtr);
        case SQL_MAX_TABLE_NAME_LEN:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 255, infoValuePtr, stringLengthPtr);
        case SQL_MAX_COLUMNS_IN_TABLE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 256, infoValuePtr, stringLengthPtr);
        case SQL_MAX_COLUMNS_IN_SELECT:
            return WriteInfoScalar<SQLUSMALLINT>(conn, 1024, infoValuePtr, stringLengthPtr);

        // Stored Procedures (not supported)
        case SQL_PROCEDURES:
            return WriteInfoString(conn, "N", infoValuePtr, bufferLength, stringLengthPtr);

        // Outer Joins (limited support)
        case SQL_OUTER_JOINS:
            return WriteInfoString(conn, "N", infoValuePtr, bufferLength, stringLengthPtr);

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

        // Case Sensitivity
        case SQL_QUOTED_IDENTIFIER_CASE:
            return WriteInfoScalar<SQLUSMALLINT>(conn, SQL_IC_SENSITIVE, infoValuePtr, stringLengthPtr);

        default:
            return conn->AddError("HYC00", 0, "Optional feature not implemented");
    }
}

} // namespace NYdb::NOdbc
