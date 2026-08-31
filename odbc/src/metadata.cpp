#include "metadata.h"

#include "utils/diag.h"
#include "utils/sql_type_map.h"

#include <algorithm>
#include <cstring>
#include <ranges>
#include <string_view>
#include <variant>

namespace NYdb::NOdbc {
namespace {

using TInfoValue = std::variant<std::string_view, SQLUSMALLINT, SQLSMALLINT, SQLUINTEGER>;

struct TInfo {
    SQLUSMALLINT Id;
    TInfoValue Value;
};

constexpr TInfo S(SQLUSMALLINT id, std::string_view value) {
    return {id, value};
}

constexpr TInfo U16(SQLUSMALLINT id, SQLUSMALLINT value) {
    return {id, value};
}

constexpr TInfo I16(SQLUSMALLINT id, SQLSMALLINT value) {
    return {id, value};
}

constexpr TInfo U32(SQLUSMALLINT id, SQLUINTEGER value) {
    return {id, value};
}

constexpr SQLUSMALLINT kMaxTableColumns = 200;
constexpr SQLUSMALLINT kMaxIndexColumns = 20;

constexpr TInfo kInfo[] = {
    S(SQL_DRIVER_NAME, "ydb-odbc"),
    S(SQL_DRIVER_VER, YDB_ODBC_DRIVER_VERSION),
    S(SQL_DRIVER_ODBC_VER, "03.00"),
    U32(SQL_ODBC_INTERFACE_CONFORMANCE, SQL_OIC_CORE),
    U16(SQL_ODBC_API_CONFORMANCE, SQL_OAC_LEVEL1),
    U16(SQL_ODBC_SAG_CLI_CONFORMANCE, SQL_OSCC_NOT_COMPLIANT),
    I16(SQL_ODBC_SQL_CONFORMANCE, SQL_OSC_MINIMUM),
    U16(SQL_MAX_TABLE_NAME_LEN, 255),
    U16(SQL_MAX_COLUMN_NAME_LEN, 255),
    U16(SQL_MAX_CATALOG_NAME_LEN, 255),
    U16(SQL_MAX_IDENTIFIER_LEN, 255),
    U16(SQL_MAX_SCHEMA_NAME_LEN, 0),
    U16(SQL_MAX_PROCEDURE_NAME_LEN, 0),
    U16(SQL_MAX_USER_NAME_LEN, 128),
    U16(SQL_MAX_DRIVER_CONNECTIONS, 0),
    U16(SQL_MAX_CONCURRENT_ACTIVITIES, 0),
    U32(SQL_MAX_STATEMENT_LEN, 0),
    U32(SQL_MAX_BINARY_LITERAL_LEN, 0),
    U32(SQL_MAX_CHAR_LITERAL_LEN, 0),
    U16(SQL_MAX_COLUMNS_IN_GROUP_BY, 0),
    U16(SQL_MAX_COLUMNS_IN_ORDER_BY, 0),
    U16(SQL_MAX_COLUMNS_IN_INDEX, kMaxIndexColumns),
    U16(SQL_MAX_COLUMNS_IN_SELECT, 0),
    U16(SQL_MAX_COLUMNS_IN_TABLE, kMaxTableColumns),
    U16(SQL_MAX_TABLES_IN_SELECT, 0),
    S(SQL_SEARCH_PATTERN_ESCAPE, "\\"),
    S(SQL_KEYWORDS, ""),
    S(SQL_SPECIAL_CHARACTERS, ""),
    U16(SQL_CONCAT_NULL_BEHAVIOR, SQL_CB_NULL),
    U16(SQL_NULL_COLLATION, SQL_NC_HIGH),
    U16(SQL_MAX_CURSOR_NAME_LEN, 128),
    S(SQL_DBMS_NAME, "YDB"),
    S(SQL_USER_NAME, ""),
    S(SQL_IDENTIFIER_QUOTE_CHAR, "`"),
    U16(SQL_IDENTIFIER_CASE, SQL_IC_SENSITIVE),
    S(SQL_CATALOG_NAME, "Y"),
    S(SQL_CATALOG_NAME_SEPARATOR, "/"),
    S(SQL_CATALOG_TERM, "path"),
    U16(SQL_CATALOG_LOCATION, SQL_CL_START),
    // YDB's database path scopes metadata and relative table names, but it is
    // not a catalog qualifier in YQL data-manipulation statements.
    U32(SQL_CATALOG_USAGE, 0),
    U32(SQL_SCHEMA_USAGE, 0),
    S(SQL_SCHEMA_TERM, ""),
    U32(SQL_ALTER_TABLE, 0),
    U16(SQL_GROUP_BY, SQL_GB_GROUP_BY_CONTAINS_SELECT),
    U16(SQL_NON_NULLABLE_COLUMNS, SQL_NNC_NON_NULL),
    S(SQL_MULT_RESULT_SETS, "N"),
    U32(SQL_DYNAMIC_CURSOR_ATTRIBUTES1, 0),
    U32(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT),
    U32(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, SQL_CA2_READ_ONLY_CONCURRENCY),
    U32(SQL_STATIC_CURSOR_ATTRIBUTES1,
        SQL_CA1_NEXT | SQL_CA1_ABSOLUTE | SQL_CA1_RELATIVE),
    U32(SQL_STATIC_CURSOR_ATTRIBUTES2,
        SQL_CA2_READ_ONLY_CONCURRENCY | SQL_CA2_MAX_ROWS_SELECT | SQL_CA2_MAX_ROWS_CATALOG),
    U32(SQL_FETCH_DIRECTION,
        SQL_FD_FETCH_NEXT | SQL_FD_FETCH_FIRST | SQL_FD_FETCH_LAST
        | SQL_FD_FETCH_PRIOR | SQL_FD_FETCH_ABSOLUTE | SQL_FD_FETCH_RELATIVE),
    U32(SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY | SQL_SO_STATIC),
    U32(SQL_SCROLL_CONCURRENCY, SQL_SCCO_READ_ONLY),
    U32(SQL_CURSOR_SENSITIVITY, SQL_INSENSITIVE),
    U16(SQL_CURSOR_COMMIT_BEHAVIOR, SQL_CB_CLOSE),
    U16(SQL_CURSOR_ROLLBACK_BEHAVIOR, SQL_CB_CLOSE),
    U16(SQL_TXN_CAPABLE, SQL_TC_DML),
    U32(SQL_DEFAULT_TXN_ISOLATION, SQL_TXN_SERIALIZABLE),
    S(SQL_PROCEDURES, "N"),
    S(SQL_OUTER_JOINS, "Y"),
    S(SQL_ORDER_BY_COLUMNS_IN_SELECT, "N"),
    U32(SQL_POSITIONED_STATEMENTS, 0),
    U32(SQL_BATCH_SUPPORT, 0),
    U32(SQL_BATCH_ROW_COUNT, 0),
    U32(SQL_PARAM_ARRAY_ROW_COUNTS, SQL_PARC_NO_BATCH),
    U32(SQL_PARAM_ARRAY_SELECTS, SQL_PAS_NO_SELECT),
    U32(SQL_BOOKMARK_PERSISTENCE, 0),
    U16(SQL_FILE_USAGE, SQL_FILE_NOT_SUPPORTED),
    U32(SQL_GETDATA_EXTENSIONS, SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER),
    U32(SQL_CONVERT_CHAR, SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
        | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR | SQL_CVT_WLONGVARCHAR),
    U32(SQL_CONVERT_VARCHAR, SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
        | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR | SQL_CVT_WLONGVARCHAR),
    U32(SQL_CONVERT_LONGVARCHAR, SQL_CVT_CHAR | SQL_CVT_VARCHAR | SQL_CVT_LONGVARCHAR
        | SQL_CVT_WCHAR | SQL_CVT_WVARCHAR | SQL_CVT_WLONGVARCHAR),
    U32(SQL_ASYNC_MODE, SQL_AM_NONE),
    U16(SQL_QUOTED_IDENTIFIER_CASE, SQL_IC_SENSITIVE),
};

struct TFunctionRange {
    SQLUSMALLINT First;
    SQLUSMALLINT Last;
};

constexpr TFunctionRange kSupportedFunctions[] = {
    {SQL_API_SQLBINDCOL, SQL_API_SQLDISCONNECT},
    {SQL_API_SQLEXECDIRECT, SQL_API_SQLFETCH},
    {SQL_API_SQLFREESTMT, SQL_API_SQLSETCURSORNAME},
    {SQL_API_SQLCOLUMNS, SQL_API_SQLDRIVERCONNECT},
    {SQL_API_SQLGETDATA, SQL_API_SQLGETINFO},
    {SQL_API_SQLGETTYPEINFO, SQL_API_SQLPUTDATA},
    {SQL_API_SQLSPECIALCOLUMNS, SQL_API_SQLTABLES},
    {SQL_API_SQLDESCRIBEPARAM, SQL_API_SQLDESCRIBEPARAM},
    {SQL_API_SQLFOREIGNKEYS, SQL_API_SQLNUMPARAMS},
    {SQL_API_SQLPRIMARYKEYS, SQL_API_SQLPRIMARYKEYS},
    {SQL_API_SQLCOLUMNPRIVILEGES, SQL_API_SQLCOLUMNPRIVILEGES},
    {SQL_API_SQLBINDPARAMETER, SQL_API_SQLBINDPARAMETER},
    {SQL_API_SQLALLOCHANDLE, SQL_API_SQLALLOCHANDLE},
    {SQL_API_SQLCLOSECURSOR, SQL_API_SQLGETENVATTR},
    {SQL_API_SQLGETSTMTATTR, SQL_API_SQLGETSTMTATTR},
    {SQL_API_SQLSETCONNECTATTR, SQL_API_SQLFETCHSCROLL},
};

template <class T>
SQLRETURN WriteInfoScalar(TConnection* connection, T value, SQLPOINTER output,
                          SQLSMALLINT* length) {
    if (!output) {
        return connection->AddError("HY009", 0, "Invalid use of null pointer");
    }
    *static_cast<T*>(output) = value;
    if (length) {
        *length = static_cast<SQLSMALLINT>(sizeof(T));
    }
    return SQL_SUCCESS;
}

SQLRETURN WriteInfo(TConnection* connection, const TInfoValue& value, SQLPOINTER output,
                    SQLSMALLINT bufferLength, SQLSMALLINT* length) {
    return std::visit(
        [&](auto item) -> SQLRETURN {
            using T = decltype(item);
            if constexpr (std::is_same_v<T, std::string_view>) {
                return Diag::WriteOdbcString(*connection, item, output, bufferLength, length);
            } else {
                return WriteInfoScalar(connection, item, output, length);
            }
        },
        value);
}

bool IsSupportedFunction(SQLUSMALLINT id) {
    return std::ranges::any_of(kSupportedFunctions, [id](const auto& range) {
        return id >= range.First && id <= range.Last;
    });
}

const TColumnMeta& GetColumn(TStatement* statement, SQLUSMALLINT number) {
    const auto& columns = statement->GetColumnMeta();
    if (number < 1 || number > columns.size()) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }
    return columns[number - 1];
}

SQLRETURN WriteAttributeNumber(TStatement* statement, SQLLEN value, SQLLEN* output) {
    if (!output) {
        return statement->AddError("HY009", 0, "Invalid use of null pointer");
    }
    *output = value;
    return SQL_SUCCESS;
}

} // namespace

SQLRETURN NMetadata::GetInfo(TConnection* connection, SQLUSMALLINT infoType,
                             SQLPOINTER infoValuePtr, SQLSMALLINT bufferLength,
                             SQLSMALLINT* stringLengthPtr) {
    switch (infoType) {
        case SQL_DBMS_VER:
            return Diag::WriteOdbcString(
                *connection, connection->GetDbmsVersion(), infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DATA_SOURCE_READ_ONLY:
            return Diag::WriteOdbcString(*connection, connection->IsDataSourceReadOnly() ? "Y" : "N",
                                         infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DATA_SOURCE_NAME:
            return Diag::WriteOdbcString(*connection, connection->GetDataSourceName(),
                                         infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_DATABASE_NAME:
            return Diag::WriteOdbcString(*connection, connection->GetDatabaseName(),
                                         infoValuePtr, bufferLength, stringLengthPtr);
        case SQL_TXN_ISOLATION_OPTION:
            return WriteInfoScalar(connection, connection->GetSupportedTxnIsolationOptions(),
                                   infoValuePtr, stringLengthPtr);
        default:
            break;
    }
    const auto info = std::ranges::find(kInfo, infoType, &TInfo::Id);
    if (info == std::end(kInfo)) {
        return connection->AddError("HYC00", 0, "Optional feature not implemented");
    }
    return WriteInfo(connection, info->Value, infoValuePtr, bufferLength, stringLengthPtr);
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
    } else if (functionId == SQL_API_ODBC3_ALL_FUNCTIONS) {
        std::memset(supportedPtr, 0, SQL_API_ODBC3_ALL_FUNCTIONS_SIZE * sizeof(SQLUSMALLINT));
        for (SQLUSMALLINT id = 0; id < SQL_API_ODBC3_ALL_FUNCTIONS_SIZE * 16; ++id) {
            if (IsSupportedFunction(id)) {
                supportedPtr[id >> 4] |= (1 << (id & 0x000F));
            }
        }
    } else {
        *supportedPtr = IsSupportedFunction(functionId) ? SQL_TRUE : SQL_FALSE;
    }
    return SQL_SUCCESS;
}

SQLRETURN NMetadata::DescribeCol(TStatement* statement, SQLUSMALLINT columnNumber,
                                 SQLCHAR* columnName, SQLSMALLINT bufferLength,
                                 SQLSMALLINT* nameLengthPtr, SQLSMALLINT* dataTypePtr,
                                 SQLULEN* columnSizePtr, SQLSMALLINT* decimalDigitsPtr,
                                 SQLSMALLINT* nullablePtr) {
    const auto& column = GetColumn(statement, columnNumber);
    const SQLRETURN result = Diag::WriteOdbcString(
        *statement, column.Name, columnName, bufferLength, nameLengthPtr);
    if (result != SQL_SUCCESS) {
        return result;
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

SQLRETURN NMetadata::ColAttribute(TStatement* statement, SQLUSMALLINT columnNumber,
                                  SQLUSMALLINT fieldIdentifier, SQLPOINTER characterAttributePtr,
                                  SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthAttributePtr,
                                  SQLLEN* numericAttributePtr) {
    const auto& column = GetColumn(statement, columnNumber);
    switch (fieldIdentifier) {
        case SQL_DESC_NAME:
        case SQL_COLUMN_NAME:
        case SQL_DESC_LABEL:
            return Diag::WriteString<Diag::EStringWriteMode::ColumnAttribute>(
                statement, column.Name, characterAttributePtr, bufferLength,
                stringLengthAttributePtr);
        case SQL_DESC_BASE_COLUMN_NAME:
        case SQL_DESC_BASE_TABLE_NAME:
        case SQL_DESC_CATALOG_NAME:
        case SQL_DESC_SCHEMA_NAME:
        case SQL_DESC_TABLE_NAME:
            return Diag::WriteString<Diag::EStringWriteMode::ColumnAttribute>(
                statement, "", characterAttributePtr, bufferLength,
                stringLengthAttributePtr);
        case SQL_DESC_TYPE_NAME: {
            const TSqlTypeSpec* spec = FindSqlTypeSpec(column.SqlType);
            return Diag::WriteString<Diag::EStringWriteMode::ColumnAttribute>(
                statement, spec ? spec->Name : "UNKNOWN", characterAttributePtr,
                bufferLength, stringLengthAttributePtr);
        }
        case SQL_DESC_TYPE:
        case SQL_DESC_CONCISE_TYPE:
            return WriteAttributeNumber(statement, column.SqlType, numericAttributePtr);
        case SQL_DESC_LENGTH:
        case SQL_DESC_DISPLAY_SIZE:
        case SQL_DESC_OCTET_LENGTH:
        case SQL_COLUMN_LENGTH:
        case SQL_DESC_PRECISION:
        case SQL_COLUMN_PRECISION:
            return WriteAttributeNumber(statement, static_cast<SQLLEN>(column.Size), numericAttributePtr);
        case SQL_DESC_SCALE:
        case SQL_COLUMN_SCALE:
            return WriteAttributeNumber(statement, column.DecimalDigits, numericAttributePtr);
        case SQL_DESC_NULLABLE:
        case SQL_COLUMN_NULLABLE:
            return WriteAttributeNumber(statement, column.Nullable, numericAttributePtr);
        case SQL_DESC_UNSIGNED:
            return WriteAttributeNumber(statement, column.Unsigned ? SQL_TRUE : SQL_FALSE,
                                        numericAttributePtr);
        case SQL_DESC_AUTO_UNIQUE_VALUE:
            return WriteAttributeNumber(statement, SQL_FALSE, numericAttributePtr);
        case SQL_DESC_CASE_SENSITIVE:
            return WriteAttributeNumber(statement,
                column.SqlType == SQL_CHAR || column.SqlType == SQL_VARCHAR
                    || column.SqlType == SQL_LONGVARCHAR || column.SqlType == SQL_WCHAR
                    || column.SqlType == SQL_WVARCHAR || column.SqlType == SQL_WLONGVARCHAR,
                numericAttributePtr);
        case SQL_DESC_FIXED_PREC_SCALE:
            return WriteAttributeNumber(statement,
                column.SqlType == SQL_DECIMAL || column.SqlType == SQL_NUMERIC,
                numericAttributePtr);
        case SQL_DESC_SEARCHABLE:
            return WriteAttributeNumber(statement, SQL_PRED_SEARCHABLE, numericAttributePtr);
        case SQL_DESC_UPDATABLE:
            return WriteAttributeNumber(statement, SQL_ATTR_READONLY, numericAttributePtr);
        default:
            return statement->AddError("HYC00", 0, "Optional feature not implemented");
    }
}

} // namespace NYdb::NOdbc
