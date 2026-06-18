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
    template<typename Handle>
    Handle* GetHandle(SQLHANDLE handle) {
        if (!handle) {
            throw NYdb::NOdbc::TOdbcException("HY000", 0, "Invalid handle", SQL_INVALID_HANDLE);
        }
        return static_cast<Handle*>(handle);
    }

}

extern "C" {

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT handleType, 
                                 SQLHANDLE inputHandle,
                                 SQLHANDLE* outputHandle) {
    if (!outputHandle) {
        return SQL_INVALID_HANDLE;
    }

    switch (handleType) {
        case SQL_HANDLE_ENV: {
            return NYdb::NOdbc::HandleOdbcExceptions(
                inputHandle,
                [&]() {
                    auto* const env = new NYdb::NOdbc::TEnvironment();
                    *outputHandle = env;
                    env->SetLastReturnCode(SQL_SUCCESS);
                    return SQL_SUCCESS;
                },
                NYdb::NOdbc::ENullInputHandlePolicy::Allow);
        }

        case SQL_HANDLE_DBC: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TEnvironment>(inputHandle, [&](auto* env) {
                auto conn = std::make_unique<NYdb::NOdbc::TConnection>();
                conn->SetEnvironment(env);
                env->RegisterConnection(conn.get());
                auto* const raw = conn.release();
                *outputHandle = raw;
                raw->SetLastReturnCode(SQL_SUCCESS);
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_STMT: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(inputHandle, [&](auto* conn) {
                auto stmt = conn->CreateStatement();
                auto* const raw = stmt.release();
                *outputHandle = raw;
                raw->SetLastReturnCode(SQL_SUCCESS);
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_DESC: {
            return NYdb::NOdbc::HandleOdbcExceptions(
                inputHandle,
                [&]() {
                    auto* const desc = new NYdb::NOdbc::TDescriptor(NYdb::NOdbc::EDescType::Explicit);
                    *outputHandle = desc;
                    desc->SetLastReturnCode(SQL_SUCCESS);
                    return SQL_SUCCESS;
                },
                NYdb::NOdbc::ENullInputHandlePolicy::Allow);
        }
        default:
            return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT handleType, SQLHANDLE handle) {
    switch (handleType) {
        case SQL_HANDLE_ENV: {
            return NYdb::NOdbc::HandleOdbcExceptionsConsuming<NYdb::NOdbc::TEnvironment>(handle, [](auto* env) {
                delete env;
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_DBC: {
            return NYdb::NOdbc::HandleOdbcExceptionsConsuming<NYdb::NOdbc::TConnection>(handle, [](auto* conn) {
                auto* env = conn->GetEnvironment();
                if (env != nullptr){
                    env->UnregisterConnection(conn);
                }
                delete conn;
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_STMT: {
            return NYdb::NOdbc::HandleOdbcExceptionsConsuming<NYdb::NOdbc::TStatement>(handle, [](auto* stmt) {
                delete stmt;
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_DESC: {
            return NYdb::NOdbc::HandleOdbcExceptionsConsuming<NYdb::NOdbc::TDescriptor>(handle, [](auto* desc) {
                delete desc;
                return SQL_SUCCESS;
            });
        }
        default:
            return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV environmentHandle,
                                SQLINTEGER attribute,
                                SQLPOINTER value,
                                SQLINTEGER stringLength) {
    auto env = static_cast<NYdb::NOdbc::TEnvironment*>(environmentHandle);
    if (!env) {
        return SQL_INVALID_HANDLE;
    }
    
    return NYdb::NOdbc::HandleOdbcExceptions(env, [&]() {
        return env->SetAttribute(attribute, value, stringLength);
    });
}

SQLRETURN SQL_API SQLGetEnvAttr(SQLHENV environmentHandle,
                                SQLINTEGER attribute,
                                SQLPOINTER value,
                                SQLINTEGER bufferLength,
                                SQLINTEGER* stringLengthPtr) {
    auto env = static_cast<NYdb::NOdbc::TEnvironment*>(environmentHandle);
    if (!env) {
        return SQL_INVALID_HANDLE;
    }
    return NYdb::NOdbc::HandleOdbcExceptions(env, [&]() {
        return env->GetAttribute(attribute, value, bufferLength, stringLengthPtr);
    });
}

SQLRETURN SQL_API SQLDriverConnect(SQLHDBC connectionHandle,
                                   SQLHWND /*WindowHandle*/,
                                   SQLCHAR* inConnectionString,
                                   SQLSMALLINT stringLength1,
                                   SQLCHAR* /*outConnectionString*/,
                                   SQLSMALLINT /*bufferLength*/,
                                   SQLSMALLINT* /*stringLength2Ptr*/,
                                   SQLUSMALLINT /*driverCompletion*/) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        return conn->DriverConnect(NYdb::NOdbc::GetString(inConnectionString, stringLength1));
    });
}

SQLRETURN SQL_API SQLConnect(SQLHDBC connectionHandle,
                             SQLCHAR* serverName, SQLSMALLINT nameLength1,
                             SQLCHAR* userName, SQLSMALLINT nameLength2,
                             SQLCHAR* authentication, SQLSMALLINT nameLength3) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        return conn->Connect(NYdb::NOdbc::GetString(serverName, nameLength1),
                            NYdb::NOdbc::GetString(userName, nameLength2),
                            NYdb::NOdbc::GetString(authentication, nameLength3));
    });
}

SQLRETURN SQL_API SQLDisconnect(SQLHDBC connectionHandle) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        return conn->Disconnect();
    });
}

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT statementHandle,
                                SQLCHAR* statementText,
                                SQLINTEGER textLength) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        auto ret = stmt->Prepare(NYdb::NOdbc::GetString(statementText, textLength));
        if (ret != SQL_SUCCESS) {
            return ret;
        }
        return stmt->Execute();
    });
}

SQLRETURN SQL_API SQLPrepare(SQLHSTMT statementHandle,
                             SQLCHAR* statementText,
                             SQLINTEGER textLength) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Prepare(NYdb::NOdbc::GetString(statementText, textLength));
    });
}

SQLRETURN SQL_API SQLExecute(SQLHSTMT statementHandle) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Execute();
    });
}

SQLRETURN SQL_API SQLFetch(SQLHSTMT statementHandle) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Fetch();
    });
}

SQLRETURN SQL_API SQLGetData(SQLHSTMT statementHandle,
                             SQLUSMALLINT columnNumber,
                             SQLSMALLINT targetType,
                             SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLenOrInd) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->GetData(columnNumber, targetType, targetValue, bufferLength, strLenOrInd);
    });
}

SQLRETURN SQL_API SQLBindCol(SQLHSTMT statementHandle,
                             SQLUSMALLINT columnNumber,
                             SQLSMALLINT targetType,
                             SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLenOrInd) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->BindCol(columnNumber, targetType, targetValue, bufferLength, strLenOrInd);
    });
}

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT handleType,
                                SQLHANDLE handle,
                                SQLSMALLINT recNumber,
                                SQLCHAR* sqlState,
                                SQLINTEGER* nativeError,
                                SQLCHAR* messageText,
                                SQLSMALLINT bufferLength,
                                SQLSMALLINT* textLength) {
    switch (handleType) {
        case SQL_HANDLE_ENV: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TEnvironment>(handle, [&](auto* env) {
                return env->GetDiagRec(recNumber, sqlState, nativeError, messageText, bufferLength, textLength);
            });
        }
        case SQL_HANDLE_DBC: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(handle, [&](auto* conn) {
                return conn->GetDiagRec(recNumber, sqlState, nativeError, messageText, bufferLength, textLength);
            });
        }
        case SQL_HANDLE_STMT: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(handle, [&](auto* stmt) {
                return stmt->GetDiagRec(recNumber, sqlState, nativeError, messageText, bufferLength, textLength);
            });
        }
        default:
            return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT handleType,
                                  SQLHANDLE handle,
                                  SQLSMALLINT recNumber,
                                  SQLSMALLINT diagIdentifier,
                                  SQLPOINTER diagInfoPtr,
                                  SQLSMALLINT bufferLength,
                                  SQLSMALLINT* stringLengthPtr) {
    switch (handleType) {
        case SQL_HANDLE_ENV: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TEnvironment>(handle, [&](auto* env) {
                return env->GetDiagField(recNumber, diagIdentifier, diagInfoPtr, bufferLength, stringLengthPtr);
            });
        }
        case SQL_HANDLE_DBC: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(handle, [&](auto* conn) {
                return conn->GetDiagField(recNumber, diagIdentifier, diagInfoPtr, bufferLength, stringLengthPtr);
            });
        }
        case SQL_HANDLE_STMT: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(handle, [&](auto* stmt) {
                return stmt->GetDiagField(recNumber, diagIdentifier, diagInfoPtr, bufferLength, stringLengthPtr);
            });
        }
        default:
            return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLBindParameter(SQLHSTMT statementHandle,
                                   SQLUSMALLINT paramNumber,
                                   SQLSMALLINT inputOutputType,
                                   SQLSMALLINT valueType,
                                   SQLSMALLINT parameterType,
                                   SQLULEN columnSize,
                                   SQLSMALLINT decimalDigits,
                                   SQLPOINTER parameterValuePtr,
                                   SQLLEN bufferLength,
                                   SQLLEN* strLenOrIndPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->BindParameter(paramNumber, inputOutputType, valueType, parameterType, columnSize, decimalDigits, parameterValuePtr, bufferLength, strLenOrIndPtr);
    });
}

SQLRETURN SQL_API SQLEndTran(SQLSMALLINT handleType, SQLHANDLE handle, SQLSMALLINT completionType) {
    switch (handleType) {
        case SQL_HANDLE_DBC: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(handle, [&](auto* conn) {
                if (completionType == SQL_COMMIT) {
                    return conn->CommitTx();
                } else if (completionType == SQL_ROLLBACK) {
                    return conn->RollbackTx();
                } else {
                    throw NYdb::NOdbc::TOdbcException("HY000", 0, "Invalid completion type");
                }
            });
        }
        case SQL_HANDLE_STMT: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(handle, [&](auto* stmt) -> SQLRETURN {
                auto conn = stmt->GetConnection();
                if (!conn) return SQL_INVALID_HANDLE;
                if (completionType == SQL_COMMIT) {
                    return conn->CommitTx();
                } else if (completionType == SQL_ROLLBACK) {
                    return conn->RollbackTx();
                } else {
                    throw NYdb::NOdbc::TOdbcException("HY000", 0, "Invalid completion type");
                }
            });
        }
        case SQL_HANDLE_ENV: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TEnvironment>(handle, [&](auto* env) -> SQLRETURN {
                return env->EndTran(completionType);
            });
        }
        default:
            return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC connectionHandle, SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        return conn->SetConnectAttr(attribute, value, stringLength);
    });
}

SQLRETURN SQL_API SQLGetConnectAttr(SQLHDBC connectionHandle, SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER bufferLength,
                                    SQLINTEGER* stringLengthPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        return conn->GetConnectAttr(attribute, value, bufferLength, stringLengthPtr);
    });
}

SQLRETURN SQL_API SQLColumns(SQLHSTMT statementHandle,
                             SQLCHAR* catalogName, SQLSMALLINT nameLength1,
                             SQLCHAR* schemaName, SQLSMALLINT nameLength2,
                             SQLCHAR* tableName, SQLSMALLINT nameLength3,
                             SQLCHAR* columnName, SQLSMALLINT nameLength4) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Columns(
            NYdb::NOdbc::GetString(catalogName, nameLength1),
            NYdb::NOdbc::GetString(schemaName, nameLength2),
            NYdb::NOdbc::GetString(tableName, nameLength3),
            NYdb::NOdbc::GetString(columnName, nameLength4));
    });
}

SQLRETURN SQL_API SQLTables(SQLHSTMT statementHandle,
                             SQLCHAR* catalogName, SQLSMALLINT nameLength1,
                             SQLCHAR* schemaName, SQLSMALLINT nameLength2,
                             SQLCHAR* tableName, SQLSMALLINT nameLength3,
                             SQLCHAR* tableType, SQLSMALLINT nameLength4) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Tables(
            NYdb::NOdbc::GetString(catalogName, nameLength1),
            NYdb::NOdbc::GetString(schemaName, nameLength2),
            NYdb::NOdbc::GetString(tableName, nameLength3),
            NYdb::NOdbc::GetString(tableType, nameLength4));
    });
}

SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT statementHandle) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Close(false);
    });
}

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT statementHandle, SQLUSMALLINT option) {
    if (option == SQL_DROP) {
        return SQLFreeHandle(SQL_HANDLE_STMT, statementHandle);
    }
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) -> SQLRETURN {
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
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        if (fetchOrientation == SQL_FETCH_NEXT) {
            return stmt->Fetch();
        } else {
            throw NYdb::NOdbc::TOdbcException("HYC00", 0, "Only SQL_FETCH_NEXT is supported");
        }
        //TODO other fetch-orientation
    });
}

SQLRETURN SQL_API SQLRowCount(SQLHSTMT statementHandle, SQLLEN* rowCount) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->RowCount(rowCount);
    });
}

SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT statementHandle, SQLSMALLINT* colCount) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->NumResultCols(colCount);
    });
}

SQLRETURN SQL_API SQLDescribeCol(
    SQLHSTMT statementHandle,
    SQLUSMALLINT columnNumber,
    SQLCHAR* columnName,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* nameLengthPtr,
    SQLSMALLINT* dataTypePtr,
    SQLULEN* columnSizePtr,
    SQLSMALLINT* decimalDigitsPtr,
    SQLSMALLINT* nullablePtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return NYdb::NOdbc::NMetadata::DescribeCol(
            stmt,
            columnNumber,
            columnName,
            bufferLength,
            nameLengthPtr,
            dataTypePtr,
            columnSizePtr,
            decimalDigitsPtr,
            nullablePtr);
    });
}

SQLRETURN SQL_API SQLMoreResults(SQLHSTMT) {
    // YDB ODBC currently exposes only one result set per statement.
    return SQL_NO_DATA;
}

SQLRETURN SQL_API SQLGetFunctions(SQLHDBC connectionHandle, SQLUSMALLINT functionId, SQLUSMALLINT* supportedPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto*) {
        return NYdb::NOdbc::NMetadata::GetFunctions(functionId, supportedPtr);
    });
}

SQLRETURN SQL_API SQLSetStmtAttr(SQLHSTMT statementHandle, SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->SetStmtAttr(attribute, value, stringLength);
    });
}

SQLRETURN SQL_API SQLGetStmtAttr(
    SQLHSTMT statementHandle,
    SQLINTEGER attribute,
    SQLPOINTER value,
    SQLINTEGER bufferLength,
    SQLINTEGER* stringLengthPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->GetStmtAttr(attribute, value, bufferLength, stringLengthPtr);
    });
}

SQLRETURN SQL_API SQLGetInfo(SQLHDBC connectionHandle,
                             SQLUSMALLINT infoType,
                             SQLPOINTER infoValuePtr,
                             SQLSMALLINT bufferLength,
                             SQLSMALLINT* stringLengthPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        return NYdb::NOdbc::NMetadata::GetInfo(conn, infoType, infoValuePtr, bufferLength, stringLengthPtr);
    });
}

SQLRETURN SQL_API SQLGetTypeInfo(SQLHSTMT statementHandle, SQLSMALLINT dataType) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->GetTypeInfo(dataType);
    });
}

SQLRETURN SQL_API SQLStatistics(SQLHSTMT statementHandle,
                                SQLCHAR* catalogName, SQLSMALLINT nameLength1,
                                SQLCHAR* schemaName, SQLSMALLINT nameLength2,
                                SQLCHAR* tableName, SQLSMALLINT nameLength3,
                                SQLUSMALLINT unique, SQLUSMALLINT reserved) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Statistics(
            NYdb::NOdbc::GetString(catalogName, nameLength1),
            NYdb::NOdbc::GetString(schemaName, nameLength2),
            NYdb::NOdbc::GetString(tableName, nameLength3),
            unique,
            reserved);
    });
}

SQLRETURN SQL_API SQLSpecialColumns(SQLHSTMT statementHandle,
                                    SQLUSMALLINT identifierType,
                                    SQLCHAR* catalogName, SQLSMALLINT nameLength1,
                                    SQLCHAR* schemaName, SQLSMALLINT nameLength2,
                                    SQLCHAR* tableName, SQLSMALLINT nameLength3,
                                    SQLUSMALLINT scope,
                                    SQLUSMALLINT nullable) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        (void)nullable;
        return stmt->SpecialColumns(
            NYdb::NOdbc::GetString(catalogName, nameLength1),
            NYdb::NOdbc::GetString(schemaName, nameLength2),
            NYdb::NOdbc::GetString(tableName, nameLength3),
            identifierType,
            scope);
    });
}

SQLRETURN SQL_API SQLColAttribute(SQLHSTMT statementHandle,
                                  SQLUSMALLINT columnNumber,
                                  SQLUSMALLINT fieldIdentifier,
                                  SQLPOINTER characterAttributePtr,
                                  SQLSMALLINT bufferLength,
                                  SQLSMALLINT* stringLengthAttributePtr,
                                  SQLLEN* numericAttributePtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return NYdb::NOdbc::NMetadata::ColAttribute(
            stmt, columnNumber, fieldIdentifier, characterAttributePtr, bufferLength,
            stringLengthAttributePtr, numericAttributePtr);
    });
}

SQLRETURN SQL_API SQLNumParams(SQLHSTMT statementHandle, SQLSMALLINT* paramCountPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->NumParams(paramCountPtr);
    });
}

SQLRETURN SQL_API SQLDescribeParam(SQLHSTMT statementHandle, SQLUSMALLINT paramNumber, SQLSMALLINT* dataTypePtr,
                                   SQLULEN* paramSizePtr, SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->DescribeParam(paramNumber, dataTypePtr, paramSizePtr, decimalDigitsPtr, nullablePtr);
    });
}

SQLRETURN SQL_API SQLParamData(SQLHSTMT statementHandle, SQLPOINTER* valuePtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->ParamData(valuePtr);
    });
}

SQLRETURN SQL_API SQLPutData(SQLHSTMT statementHandle, SQLPOINTER data, SQLLEN strLenOrInd) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->PutData(data, strLenOrInd);
    });
}

SQLRETURN SQL_API SQLCancel(SQLHSTMT statementHandle) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->Cancel();
    });
}

SQLRETURN SQL_API SQLNativeSql(SQLHDBC connectionHandle,
                               SQLCHAR* inNativeSql,
                               SQLINTEGER textLength1,
                               SQLCHAR* outNativeSql,
                               SQLINTEGER bufferLength,
                               SQLINTEGER* outLengthPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        const std::string inSql = textLength1 == SQL_NTS
            ? reinterpret_cast<const char*>(inNativeSql)
            : NYdb::NOdbc::GetString(inNativeSql, static_cast<SQLSMALLINT>(textLength1));
        return conn->NativeSql(inSql, outNativeSql, bufferLength, outLengthPtr);
    });
}

SQLRETURN SQL_API SQLSetCursorName(SQLHSTMT statementHandle, SQLCHAR* cursorName, SQLSMALLINT nameLength) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->SetCursorName(NYdb::NOdbc::GetString(cursorName, nameLength));
    });
}

SQLRETURN SQL_API SQLGetCursorName(SQLHSTMT statementHandle,
                                   SQLCHAR* cursorName,
                                   SQLSMALLINT bufferLength,
                                   SQLSMALLINT* nameLengthPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->GetCursorName(cursorName, bufferLength, nameLengthPtr);
    });
}

SQLRETURN SQL_API SQLPrimaryKeys(SQLHSTMT statementHandle,
                                 SQLCHAR* catalogName, SQLSMALLINT nameLength1,
                                 SQLCHAR* schemaName, SQLSMALLINT nameLength2,
                                 SQLCHAR* tableName, SQLSMALLINT nameLength3) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->PrimaryKeys(
            NYdb::NOdbc::GetString(catalogName, nameLength1),
            NYdb::NOdbc::GetString(schemaName, nameLength2),
            NYdb::NOdbc::GetString(tableName, nameLength3));
    });
}

SQLRETURN SQL_API SQLForeignKeys(SQLHSTMT statementHandle,
                                 SQLCHAR* pkCatalogName, SQLSMALLINT nameLength1,
                                 SQLCHAR* pkSchemaName, SQLSMALLINT nameLength2,
                                 SQLCHAR* pkTableName, SQLSMALLINT nameLength3,
                                 SQLCHAR* fkCatalogName, SQLSMALLINT nameLength4,
                                 SQLCHAR* fkSchemaName, SQLSMALLINT nameLength5,
                                 SQLCHAR* fkTableName, SQLSMALLINT nameLength6) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) {
        return stmt->ForeignKeys(
            NYdb::NOdbc::GetString(pkCatalogName, nameLength1),
            NYdb::NOdbc::GetString(pkSchemaName, nameLength2),
            NYdb::NOdbc::GetString(pkTableName, nameLength3),
            NYdb::NOdbc::GetString(fkCatalogName, nameLength4),
            NYdb::NOdbc::GetString(fkSchemaName, nameLength5),
            NYdb::NOdbc::GetString(fkTableName, nameLength6));
    });
}

SQLRETURN SQL_API SQLGetDescField(SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier,
                                  SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TDescriptor>(descriptorHandle, [&](auto* desc) {
        return desc->GetDescField(recNumber, fieldIdentifier, value, bufferLength, stringLengthPtr);
    });
}

SQLRETURN SQL_API SQLGetDescRec(SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLCHAR* name,
                                SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthPtr, SQLSMALLINT* typePtr,
                                SQLSMALLINT* subTypePtr, SQLLEN* lengthPtr, SQLSMALLINT* precisionPtr,
                                SQLSMALLINT* scalePtr, SQLSMALLINT* nullablePtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TDescriptor>(descriptorHandle, [&](auto* desc) {
        return desc->GetDescRec(recNumber, name, bufferLength, stringLengthPtr, typePtr, subTypePtr,
                                lengthPtr, precisionPtr, scalePtr, nullablePtr);
    });
}

SQLRETURN SQL_API SQLSetDescField(SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier,
                                  SQLPOINTER value, SQLINTEGER bufferLength) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TDescriptor>(descriptorHandle, [&](auto* desc) {
        return desc->SetDescField(recNumber, fieldIdentifier, value, bufferLength);
    });
}

SQLRETURN SQL_API SQLSetDescRec(SQLHDESC descriptorHandle, SQLSMALLINT recNumber, SQLSMALLINT type,
                                SQLSMALLINT subType, SQLLEN length, SQLSMALLINT precision, SQLSMALLINT scale,
                                SQLPOINTER dataPtr, SQLLEN* stringLengthPtr, SQLLEN* indicatorPtr) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TDescriptor>(descriptorHandle, [&](auto* desc) {
        return desc->SetDescRec(recNumber, type, subType, length, precision, scale, dataPtr,
                                stringLengthPtr, indicatorPtr);
    });
}

SQLRETURN SQL_API SQLCopyDesc(SQLHDESC sourceDesc, SQLHDESC targetDesc) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TDescriptor>(sourceDesc, [&](auto* src) {
        return src->CopyDesc(NYdb::NOdbc::TDescriptor::FromHandle(targetDesc));
    });
}

}
