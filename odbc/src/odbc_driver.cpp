#include "environment.h"
#include "connection.h"
#include "statement.h"
#include "metadata.h"
#include "descriptor.h"

#include "utils/util.h"
#include "utils/error_manager.h"

#include <sql.h>
#include <sqlext.h>

namespace {
    namespace Odbc = NYdb::NOdbc;
    using Odbc::TConnection;
    using Odbc::TDescriptor;
    using Odbc::TEnvironment;
    using Odbc::TStatement;

    template<Odbc::ECallMode Mode = Odbc::ECallMode::Ordinary, class Handle, class Fn>
    SQLRETURN Call(SQLHANDLE handle, Fn&& fn) {
        return Odbc::CallOdbc<Mode, Handle>(handle, std::forward<Fn>(fn));
    }

    template<class Char>
    struct TLazyText {
        Char* Value;
        SQLINTEGER Length;
        std::string Resolve() const {
            return Odbc::GetString(Value, Length);
        }
    };

    template<class T>
    decltype(auto) Resolve(T& value) {
        if constexpr (requires { value.Resolve(); }) {
            return value.Resolve();
        } else {
            return (value);
        }
    }

    template<class Char>
    TLazyText<Char> Text(Char* value, SQLINTEGER length) {
        return {value, length};
    }

    template<class Handle, auto Method, class... Args>
    SQLRETURN Forward(SQLHANDLE handle, Args&&... args) {
        return Call<Odbc::ECallMode::Ordinary, Handle>(handle, [&](Handle* value) {
            return Odbc::InvokeOdbc(Method, value, Resolve(args)...);
        });
    }

    template<bool Execute, class Char>
    SQLRETURN Prepare(SQLHSTMT handle, Char* sql, SQLINTEGER length) {
        return Call<Odbc::ECallMode::Ordinary, TStatement>(handle, [&](auto* statement) {
            const SQLRETURN result = statement->Prepare(Odbc::GetString(sql, length));
            return Execute && result == SQL_SUCCESS ? statement->Execute() : result;
        });
    }

    template<class Handle>
    SQLRETURN Free(SQLHANDLE handle) {
        return Call<Odbc::ECallMode::Consuming, Handle>(handle, [](Handle* value) {
            if constexpr (std::is_same_v<Handle, TEnvironment>) {
                if (!value->GetConnectionsSnapshot().empty()) {
                    return value->AddError("HY010", 0, "Connection handles are still allocated");
                }
            } else if constexpr (std::is_same_v<Handle, TConnection>) {
                if (value->HasChildren()) {
                    return value->AddError("HY010", 0, "Statement or descriptor handles are still allocated");
                }
                if (auto* environment = value->GetEnvironment()) {
                    environment->UnregisterConnection(value);
                }
            } else if constexpr (std::is_same_v<Handle, TDescriptor>) {
                if (value->GetDescType() != Odbc::EDescType::Explicit) {
                    return value->AddError("HY017", 0, "Invalid use of an automatically allocated descriptor handle");
                }
            }
            delete value;
            return static_cast<SQLRETURN>(SQL_SUCCESS);
        });
    }

    template<class Handle>
    SQLRETURN Allocate(SQLHANDLE parentHandle, SQLHANDLE* output) {
        if constexpr (std::is_same_v<Handle, TEnvironment>) {
            return Odbc::CallOdbcUnchecked(parentHandle, [&] {
                *output = new TEnvironment();
                static_cast<TEnvironment*>(*output)->SetLastReturnCode(SQL_SUCCESS);
                return SQL_SUCCESS;
            }, Odbc::ENullInputHandlePolicy::Allow);
        } else {
            using Parent = std::conditional_t<std::is_same_v<Handle, TConnection>, TEnvironment, TConnection>;
            return Call<Odbc::ECallMode::Ordinary, Parent>(parentHandle, [&](Parent* parent) {
                std::unique_ptr<Handle> value;
                if constexpr (std::is_same_v<Handle, TConnection>) {
                    value = std::make_unique<TConnection>();
                    value->SetEnvironment(parent);
                    parent->RegisterConnection(value.get());
                } else if constexpr (std::is_same_v<Handle, TStatement>) {
                    value = parent->CreateStatement();
                } else {
                    value = std::make_unique<TDescriptor>(Odbc::EDescType::Explicit, parent);
                }
                *output = value.release();
                static_cast<Handle*>(*output)->SetLastReturnCode(SQL_SUCCESS);
                return SQL_SUCCESS;
            });
        }
    }

    template<NYdb::NOdbc::ECallMode Mode, class Fn>
    SQLRETURN VisitHandle(SQLSMALLINT type, SQLHANDLE handle, Fn&& fn) {
        switch (type) {
            case SQL_HANDLE_ENV: return Call<Mode, TEnvironment>(handle, fn);
            case SQL_HANDLE_DBC: return Call<Mode, TConnection>(handle, fn);
            case SQL_HANDLE_STMT: return Call<Mode, TStatement>(handle, fn);
            case SQL_HANDLE_DESC: return Call<Mode, TDescriptor>(handle, fn);
            default: return SQL_ERROR;
        }
    }

}

extern "C" {

#define ODBC_FORWARD(NAME, HANDLE, METHOD, SIGNATURE, ARGUMENTS) \
    SQLRETURN SQL_API NAME SIGNATURE {                           \
        return Forward<HANDLE, &METHOD> ARGUMENTS;               \
    }

#define ODBC_PREPARE(NAME, CHAR, EXECUTE)                                      \
    SQLRETURN SQL_API NAME(                                                    \
        SQLHSTMT statementHandle, CHAR* statementText, SQLINTEGER textLength) { \
        return Prepare<EXECUTE>(statementHandle, statementText, textLength);    \
    }

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT handleType, 
                                 SQLHANDLE inputHandle,
                                 SQLHANDLE* outputHandle) {
    if (!outputHandle) {
        return SQL_INVALID_HANDLE;
    }

    switch (handleType) {
        case SQL_HANDLE_ENV: return Allocate<TEnvironment>(inputHandle, outputHandle);
        case SQL_HANDLE_DBC: return Allocate<TConnection>(inputHandle, outputHandle);
        case SQL_HANDLE_STMT: return Allocate<TStatement>(inputHandle, outputHandle);
        case SQL_HANDLE_DESC: return Allocate<TDescriptor>(inputHandle, outputHandle);
        default: return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT handleType, SQLHANDLE handle) {
    switch (handleType) {
        case SQL_HANDLE_ENV: return Free<TEnvironment>(handle);
        case SQL_HANDLE_DBC: return Free<TConnection>(handle);
        case SQL_HANDLE_STMT: return Free<TStatement>(handle);
        case SQL_HANDLE_DESC: return Free<TDescriptor>(handle);
        default: return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV environmentHandle,
                                SQLINTEGER attribute,
                                SQLPOINTER value,
                                SQLINTEGER stringLength) {
    return Odbc::CallOdbcUnchecked(environmentHandle, [&] {
        return static_cast<TEnvironment*>(environmentHandle)->SetAttribute(attribute, value, stringLength);
    });
}

SQLRETURN SQL_API SQLGetEnvAttr(SQLHENV environmentHandle,
                                SQLINTEGER attribute,
                                SQLPOINTER value,
                                SQLINTEGER bufferLength,
                                SQLINTEGER* stringLengthPtr) {
    return Odbc::CallOdbcUnchecked(environmentHandle, [&] {
        return static_cast<TEnvironment*>(environmentHandle)->GetAttribute(
            attribute, value, bufferLength, stringLengthPtr);
    });
}

ODBC_FORWARD(SQLDriverConnect, TConnection, TConnection::DriverConnect,
    (SQLHDBC connectionHandle, SQLHWND, SQLCHAR* inConnectionString,
     SQLSMALLINT stringLength1, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLUSMALLINT),
    (connectionHandle, Text(inConnectionString, stringLength1)))

ODBC_FORWARD(SQLConnect, TConnection, TConnection::Connect,
    (SQLHDBC connectionHandle, SQLCHAR* serverName, SQLSMALLINT nameLength1,
     SQLCHAR* userName, SQLSMALLINT nameLength2,
     SQLCHAR* authentication, SQLSMALLINT nameLength3),
    (connectionHandle, Text(serverName, nameLength1), Text(userName, nameLength2),
     Text(authentication, nameLength3)))

ODBC_FORWARD(SQLDisconnect, TConnection, TConnection::Disconnect,
    (SQLHDBC connectionHandle), (connectionHandle))

ODBC_PREPARE(SQLExecDirect, SQLCHAR, true)
ODBC_PREPARE(SQLExecDirectW, SQLWCHAR, true)
ODBC_PREPARE(SQLPrepare, SQLCHAR, false)
ODBC_PREPARE(SQLPrepareW, SQLWCHAR, false)

ODBC_FORWARD(SQLExecute, TStatement, TStatement::Execute,
    (SQLHSTMT statementHandle), (statementHandle))
ODBC_FORWARD(SQLFetch, TStatement, TStatement::Fetch,
    (SQLHSTMT statementHandle), (statementHandle))

ODBC_FORWARD(SQLGetData, TStatement, TStatement::GetData,
    (SQLHSTMT statementHandle, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
     SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd),
    (statementHandle, columnNumber, targetType, targetValue, bufferLength, strLenOrInd))

ODBC_FORWARD(SQLBindCol, TStatement, TStatement::BindCol,
    (SQLHSTMT statementHandle, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
     SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd),
    (statementHandle, columnNumber, targetType, targetValue, bufferLength, strLenOrInd))

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT handleType,
                                SQLHANDLE handle,
                                SQLSMALLINT recNumber,
                                SQLCHAR* sqlState,
                                SQLINTEGER* nativeError,
                                SQLCHAR* messageText,
                                SQLSMALLINT bufferLength,
                                SQLSMALLINT* textLength) {
    return VisitHandle<NYdb::NOdbc::ECallMode::Diagnostic>(handleType, handle, [&](auto* typed) {
        return typed->GetDiagRec(
            recNumber, sqlState, nativeError, messageText, bufferLength, textLength);
    });
}

SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT handleType,
                                  SQLHANDLE handle,
                                  SQLSMALLINT recNumber,
                                  SQLSMALLINT diagIdentifier,
                                  SQLPOINTER diagInfoPtr,
                                  SQLSMALLINT bufferLength,
                                  SQLSMALLINT* stringLengthPtr) {
    return VisitHandle<NYdb::NOdbc::ECallMode::Diagnostic>(handleType, handle, [&](auto* typed) {
        return typed->GetDiagField(
            recNumber, diagIdentifier, diagInfoPtr, bufferLength, stringLengthPtr);
    });
}

ODBC_FORWARD(SQLBindParameter, TStatement, TStatement::BindParameter,
    (SQLHSTMT statementHandle, SQLUSMALLINT paramNumber, SQLSMALLINT inputOutputType,
     SQLSMALLINT valueType, SQLSMALLINT parameterType, SQLULEN columnSize,
     SQLSMALLINT decimalDigits, SQLPOINTER parameterValuePtr, SQLLEN bufferLength,
     SQLLEN* strLenOrIndPtr),
    (statementHandle, paramNumber, inputOutputType, valueType, parameterType, columnSize,
     decimalDigits, parameterValuePtr, bufferLength, strLenOrIndPtr))

SQLRETURN SQL_API SQLEndTran(SQLSMALLINT handleType, SQLHANDLE handle, SQLSMALLINT completionType) {
    if (handleType == SQL_HANDLE_ENV) {
        return Forward<TEnvironment, &TEnvironment::EndTran>(handle, completionType);
    }
    if (handleType == SQL_HANDLE_DBC) {
        return Call<Odbc::ECallMode::Ordinary, TConnection>(handle, [&](auto* connection) {
            if (completionType == SQL_COMMIT) {
                return connection->CommitTx();
            }
            if (completionType == SQL_ROLLBACK) {
                return connection->RollbackTx();
            }
            throw Odbc::TOdbcException("HY012", 0, "Invalid transaction operation code");
        });
    }
    return SQL_INVALID_HANDLE;
}

ODBC_FORWARD(SQLSetConnectAttr, TConnection, TConnection::SetConnectAttr,
    (SQLHDBC connectionHandle, SQLINTEGER attribute, SQLPOINTER value,
     SQLINTEGER stringLength),
    (connectionHandle, attribute, value, stringLength))

ODBC_FORWARD(SQLGetConnectAttr, TConnection, TConnection::GetConnectAttr,
    (SQLHDBC connectionHandle, SQLINTEGER attribute, SQLPOINTER value,
     SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr),
    (connectionHandle, attribute, value, bufferLength, stringLengthPtr))

ODBC_FORWARD(SQLColumns, TStatement, TStatement::Columns,
    (SQLHSTMT statementHandle, SQLCHAR* catalogName, SQLSMALLINT nameLength1,
     SQLCHAR* schemaName, SQLSMALLINT nameLength2, SQLCHAR* tableName,
     SQLSMALLINT nameLength3, SQLCHAR* columnName, SQLSMALLINT nameLength4),
    (statementHandle, Text(catalogName, nameLength1), Text(schemaName, nameLength2),
     Text(tableName, nameLength3), Text(columnName, nameLength4)))

ODBC_FORWARD(SQLTables, TStatement, TStatement::Tables,
    (SQLHSTMT statementHandle, SQLCHAR* catalogName, SQLSMALLINT nameLength1,
     SQLCHAR* schemaName, SQLSMALLINT nameLength2, SQLCHAR* tableName,
     SQLSMALLINT nameLength3, SQLCHAR* tableType, SQLSMALLINT nameLength4),
    (statementHandle, Text(catalogName, nameLength1), Text(schemaName, nameLength2),
     Text(tableName, nameLength3), Text(tableType, nameLength4)))

ODBC_FORWARD(SQLCloseCursor, TStatement, TStatement::Close,
    (SQLHSTMT statementHandle), (statementHandle, false))

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT statementHandle, SQLUSMALLINT option) {
    if (option == SQL_DROP) {
        return SQLFreeHandle(SQL_HANDLE_STMT, statementHandle);
    }
    return Call<Odbc::ECallMode::Ordinary, TStatement>(statementHandle, [&](auto* stmt) -> SQLRETURN {
        switch (option) {
            case SQL_CLOSE:
                return stmt->Close(true);
            case SQL_UNBIND:
                stmt->UnbindColumns();
                return SQL_SUCCESS;
            case SQL_RESET_PARAMS:
                stmt->ResetParams();
                return SQL_SUCCESS;
            default:
                throw NYdb::NOdbc::TOdbcException("HY000", 0, "Invalid option");
        }
    });
}

SQLRETURN SQL_API SQLFetchScroll(SQLHSTMT statementHandle, SQLSMALLINT fetchOrientation, SQLLEN fetchOffset) {
    return Call<Odbc::ECallMode::Ordinary, TStatement>(statementHandle, [&](auto* stmt) {
        if (fetchOrientation == SQL_FETCH_NEXT) {
            return stmt->Fetch();
        } else {
            throw NYdb::NOdbc::TOdbcException("HYC00", 0, "Only SQL_FETCH_NEXT is supported");
        }
        //TODO other fetch-orientation
    });
}

ODBC_FORWARD(SQLRowCount, TStatement, TStatement::RowCount,
    (SQLHSTMT statementHandle, SQLLEN* rowCount), (statementHandle, rowCount))

ODBC_FORWARD(SQLNumResultCols, TStatement, TStatement::NumResultCols,
    (SQLHSTMT statementHandle, SQLSMALLINT* colCount), (statementHandle, colCount))

ODBC_FORWARD(SQLDescribeCol, TStatement, Odbc::NMetadata::DescribeCol,
    (SQLHSTMT statementHandle, SQLUSMALLINT columnNumber, SQLCHAR* columnName,
     SQLSMALLINT bufferLength, SQLSMALLINT* nameLengthPtr, SQLSMALLINT* dataTypePtr,
     SQLULEN* columnSizePtr, SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr),
    (statementHandle, columnNumber, columnName, bufferLength, nameLengthPtr, dataTypePtr,
     columnSizePtr, decimalDigitsPtr, nullablePtr))

SQLRETURN SQL_API SQLMoreResults(SQLHSTMT) {
    // YDB ODBC currently exposes only one result set per statement.
    return SQL_NO_DATA;
}

SQLRETURN SQL_API SQLGetFunctions(SQLHDBC connectionHandle, SQLUSMALLINT functionId, SQLUSMALLINT* supportedPtr) {
    return Call<Odbc::ECallMode::Ordinary, TConnection>(connectionHandle, [&](auto*) {
        return NYdb::NOdbc::NMetadata::GetFunctions(functionId, supportedPtr);
    });
}

ODBC_FORWARD(SQLSetStmtAttr, TStatement, TStatement::SetStmtAttr,
    (SQLHSTMT statementHandle, SQLINTEGER attribute, SQLPOINTER value,
     SQLINTEGER stringLength),
    (statementHandle, attribute, value, stringLength))

ODBC_FORWARD(SQLGetStmtAttr, TStatement, TStatement::GetStmtAttr,
    (SQLHSTMT statementHandle, SQLINTEGER attribute, SQLPOINTER value,
     SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr),
    (statementHandle, attribute, value, bufferLength, stringLengthPtr))

ODBC_FORWARD(SQLGetInfo, TConnection, Odbc::NMetadata::GetInfo,
    (SQLHDBC connectionHandle, SQLUSMALLINT infoType, SQLPOINTER infoValuePtr,
     SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthPtr),
    (connectionHandle, infoType, infoValuePtr, bufferLength, stringLengthPtr))

ODBC_FORWARD(SQLGetTypeInfo, TStatement, TStatement::GetTypeInfo,
    (SQLHSTMT statementHandle, SQLSMALLINT dataType), (statementHandle, dataType))

ODBC_FORWARD(SQLStatistics, TStatement, TStatement::Statistics,
    (SQLHSTMT statementHandle, SQLCHAR* catalogName, SQLSMALLINT nameLength1,
     SQLCHAR* schemaName, SQLSMALLINT nameLength2, SQLCHAR* tableName,
     SQLSMALLINT nameLength3, SQLUSMALLINT unique, SQLUSMALLINT reserved),
    (statementHandle, Text(catalogName, nameLength1), Text(schemaName, nameLength2),
     Text(tableName, nameLength3), unique, reserved))

ODBC_FORWARD(SQLSpecialColumns, TStatement, TStatement::SpecialColumns,
    (SQLHSTMT statementHandle, SQLUSMALLINT identifierType, SQLCHAR* catalogName,
     SQLSMALLINT nameLength1, SQLCHAR* schemaName, SQLSMALLINT nameLength2,
     SQLCHAR* tableName, SQLSMALLINT nameLength3, SQLUSMALLINT scope, SQLUSMALLINT),
    (statementHandle, Text(catalogName, nameLength1), Text(schemaName, nameLength2),
     Text(tableName, nameLength3), identifierType, scope))

ODBC_FORWARD(SQLColAttribute, TStatement, Odbc::NMetadata::ColAttribute,
    (SQLHSTMT statementHandle, SQLUSMALLINT columnNumber, SQLUSMALLINT fieldIdentifier,
     SQLPOINTER characterAttributePtr, SQLSMALLINT bufferLength,
     SQLSMALLINT* stringLengthAttributePtr, SQLLEN* numericAttributePtr),
    (statementHandle, columnNumber, fieldIdentifier, characterAttributePtr, bufferLength,
     stringLengthAttributePtr, numericAttributePtr))

ODBC_FORWARD(SQLNumParams, TStatement, TStatement::NumParams,
    (SQLHSTMT statementHandle, SQLSMALLINT* paramCountPtr),
    (statementHandle, paramCountPtr))

ODBC_FORWARD(SQLDescribeParam, TStatement, TStatement::DescribeParam,
    (SQLHSTMT statementHandle, SQLUSMALLINT paramNumber, SQLSMALLINT* dataTypePtr,
     SQLULEN* paramSizePtr, SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr),
    (statementHandle, paramNumber, dataTypePtr, paramSizePtr, decimalDigitsPtr, nullablePtr))

ODBC_FORWARD(SQLParamData, TStatement, TStatement::ParamData,
    (SQLHSTMT statementHandle, SQLPOINTER* valuePtr), (statementHandle, valuePtr))
ODBC_FORWARD(SQLPutData, TStatement, TStatement::PutData,
    (SQLHSTMT statementHandle, SQLPOINTER data, SQLLEN strLenOrInd),
    (statementHandle, data, strLenOrInd))
ODBC_FORWARD(SQLCancel, TStatement, TStatement::Cancel,
    (SQLHSTMT statementHandle), (statementHandle))

ODBC_FORWARD(SQLNativeSql, TConnection, TConnection::NativeSql,
    (SQLHDBC connectionHandle, SQLCHAR* inNativeSql, SQLINTEGER textLength1,
     SQLCHAR* outNativeSql, SQLINTEGER bufferLength, SQLINTEGER* outLengthPtr),
    (connectionHandle,
     Text(inNativeSql, textLength1 == SQL_NTS ? SQL_NTS : static_cast<SQLSMALLINT>(textLength1)),
     outNativeSql, bufferLength, outLengthPtr))

ODBC_FORWARD(SQLSetCursorName, TStatement, TStatement::SetCursorName,
    (SQLHSTMT statementHandle, SQLCHAR* cursorName, SQLSMALLINT nameLength),
    (statementHandle, Text(cursorName, nameLength)))

ODBC_FORWARD(SQLGetCursorName, TStatement, TStatement::GetCursorName,
    (SQLHSTMT statementHandle, SQLCHAR* cursorName, SQLSMALLINT bufferLength,
     SQLSMALLINT* nameLengthPtr),
    (statementHandle, cursorName, bufferLength, nameLengthPtr))

ODBC_FORWARD(SQLPrimaryKeys, TStatement, TStatement::PrimaryKeys,
    (SQLHSTMT statementHandle, SQLCHAR* catalogName, SQLSMALLINT nameLength1,
     SQLCHAR* schemaName, SQLSMALLINT nameLength2,
     SQLCHAR* tableName, SQLSMALLINT nameLength3),
    (statementHandle, Text(catalogName, nameLength1), Text(schemaName, nameLength2),
     Text(tableName, nameLength3)))

ODBC_FORWARD(SQLForeignKeys, TStatement, TStatement::ForeignKeys,
    (SQLHSTMT statementHandle, SQLCHAR* pkCatalogName, SQLSMALLINT nameLength1,
     SQLCHAR* pkSchemaName, SQLSMALLINT nameLength2,
     SQLCHAR* pkTableName, SQLSMALLINT nameLength3,
     SQLCHAR* fkCatalogName, SQLSMALLINT nameLength4,
     SQLCHAR* fkSchemaName, SQLSMALLINT nameLength5,
     SQLCHAR* fkTableName, SQLSMALLINT nameLength6),
    (statementHandle, Text(pkCatalogName, nameLength1), Text(pkSchemaName, nameLength2),
     Text(pkTableName, nameLength3), Text(fkCatalogName, nameLength4),
     Text(fkSchemaName, nameLength5), Text(fkTableName, nameLength6)))

ODBC_FORWARD(SQLGetDescField, TDescriptor, TDescriptor::GetDescField,
    (SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier,
     SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr),
    (descriptorHandle, recNumber, fieldIdentifier, value, bufferLength, stringLengthPtr))

ODBC_FORWARD(SQLGetDescRec, TDescriptor, TDescriptor::GetDescRec,
    (SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLCHAR* name,
     SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthPtr, SQLSMALLINT* typePtr,
     SQLSMALLINT* subTypePtr, SQLLEN* lengthPtr, SQLSMALLINT* precisionPtr,
     SQLSMALLINT* scalePtr, SQLSMALLINT* nullablePtr),
    (descriptorHandle, recNumber, name, bufferLength, stringLengthPtr, typePtr, subTypePtr,
     lengthPtr, precisionPtr, scalePtr, nullablePtr))

ODBC_FORWARD(SQLSetDescField, TDescriptor, TDescriptor::SetDescField,
    (SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier,
     SQLPOINTER value, SQLINTEGER bufferLength),
    (descriptorHandle, recNumber, fieldIdentifier, value, bufferLength))

ODBC_FORWARD(SQLSetDescRec, TDescriptor, TDescriptor::SetDescRec,
    (SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLSMALLINT type,
     SQLSMALLINT subType, SQLLEN length, SQLSMALLINT precision, SQLSMALLINT scale,
     SQLPOINTER dataPtr, SQLLEN* stringLengthPtr, SQLLEN* indicatorPtr),
    (descriptorHandle, recNumber, type, subType, length, precision, scale, dataPtr,
     stringLengthPtr, indicatorPtr))

SQLRETURN SQL_API SQLCopyDesc(SQLHDESC sourceDesc, SQLHDESC targetDesc) {
    return Call<Odbc::ECallMode::Ordinary, TDescriptor>(sourceDesc, [&](auto* src) {
        return src->CopyDesc(NYdb::NOdbc::TDescriptor::FromHandle(targetDesc));
    });
}

#undef ODBC_PREPARE
#undef ODBC_FORWARD

}
