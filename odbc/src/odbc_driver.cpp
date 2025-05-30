#include "environment.h"
#include "connection.h"
#include "statement.h"

#include "utils/util.h"

#include <sql.h>
#include <sqlext.h>

extern "C" {

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT handleType, 
                                 SQLHANDLE inputHandle,
                                 SQLHANDLE* outputHandle) {
    if (!outputHandle) {
        return SQL_INVALID_HANDLE;
    }
    
    try {
        switch (handleType) {
            case SQL_HANDLE_ENV: {
                if (inputHandle != SQL_NULL_HANDLE) {
                    return SQL_INVALID_HANDLE;
                }

                *outputHandle = new NYdb::NOdbc::TEnvironment();
                return SQL_SUCCESS;
            }

            case SQL_HANDLE_DBC: {
                if (!inputHandle) {
                    return SQL_INVALID_HANDLE;
                }

                *outputHandle = new NYdb::NOdbc::TConnection();
                return SQL_SUCCESS;
            }

            case SQL_HANDLE_STMT: {
                auto conn = static_cast<NYdb::NOdbc::TConnection*>(inputHandle);
                if (!conn) {
                    return SQL_INVALID_HANDLE;
                }
                auto stmt = conn->CreateStatement();
                *outputHandle = stmt.release();
                return SQL_SUCCESS;
            }

            default:
                return SQL_ERROR;
        }
    } catch (...) {
        return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT handleType, SQLHANDLE handle) {
    if (!handle) {
        return SQL_INVALID_HANDLE;
    }
    
    try {
        switch (handleType) {
            case SQL_HANDLE_ENV: {
                auto env = static_cast<NYdb::NOdbc::TEnvironment*>(handle);
                delete env;
                return SQL_SUCCESS;
            }
            
            case SQL_HANDLE_DBC: {
                auto conn = static_cast<NYdb::NOdbc::TConnection*>(handle);
                delete conn;
                return SQL_SUCCESS;
            }
            
            case SQL_HANDLE_STMT: {
                auto stmt = static_cast<NYdb::NOdbc::TStatement*>(handle);
                if (stmt->GetConnection()) {
                    stmt->GetConnection()->RemoveStatement(stmt);
                }
                delete stmt;
                return SQL_SUCCESS;
            }
            
            default:
                return SQL_ERROR;
        }
    } catch (...) {
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
    
    return env->SetAttribute(attribute, value, stringLength);
}

SQLRETURN SQL_API SQLDriverConnect(SQLHDBC connectionHandle,
                                   SQLHWND /*WindowHandle*/,
                                   SQLCHAR* inConnectionString,
                                   SQLSMALLINT stringLength1,
                                   SQLCHAR* /*outConnectionString*/,
                                   SQLSMALLINT /*bufferLength*/,
                                   SQLSMALLINT* /*stringLength2Ptr*/,
                                   SQLUSMALLINT /*driverCompletion*/) {
    auto conn = static_cast<NYdb::NOdbc::TConnection*>(connectionHandle);
    if (!conn) {
        return SQL_INVALID_HANDLE;
    }

    return conn->DriverConnect(NYdb::NOdbc::GetString(inConnectionString, stringLength1));
}

SQLRETURN SQL_API SQLConnect(SQLHDBC connectionHandle,
                             SQLCHAR* serverName, SQLSMALLINT nameLength1,
                             SQLCHAR* userName, SQLSMALLINT nameLength2,
                             SQLCHAR* authentication, SQLSMALLINT nameLength3) {
    auto conn = static_cast<NYdb::NOdbc::TConnection*>(connectionHandle);
    if (!conn) {
        return SQL_INVALID_HANDLE;
    }

    return conn->Connect(NYdb::NOdbc::GetString(serverName, nameLength1),
                         NYdb::NOdbc::GetString(userName, nameLength2),
                         NYdb::NOdbc::GetString(authentication, nameLength3));
}

SQLRETURN SQL_API SQLDisconnect(SQLHDBC connectionHandle) {
    auto conn = static_cast<NYdb::NOdbc::TConnection*>(connectionHandle);
    if (!conn) {
        return SQL_INVALID_HANDLE;
    }
    
    return conn->Disconnect();
}

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT statementHandle,
                                SQLCHAR* statementText,
                                SQLINTEGER textLength) {
    auto stmt = static_cast<NYdb::NOdbc::TStatement*>(statementHandle);
    if (!stmt) {
        return SQL_INVALID_HANDLE;
    }

    return stmt->ExecDirect(NYdb::NOdbc::GetString(statementText, textLength));
}

SQLRETURN SQL_API SQLFetch(SQLHSTMT statementHandle) {
    auto stmt = static_cast<NYdb::NOdbc::TStatement*>(statementHandle);
    if (!stmt) {
        return SQL_INVALID_HANDLE;
    }
    
    return stmt->Fetch();
}

SQLRETURN SQL_API SQLGetData(SQLHSTMT statementHandle,
                             SQLUSMALLINT columnNumber,
                             SQLSMALLINT targetType,
                             SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLenOrInd) {
    auto stmt = static_cast<NYdb::NOdbc::TStatement*>(statementHandle);
    if (!stmt) {
        return SQL_INVALID_HANDLE;
    }
    
    return stmt->GetData(columnNumber, targetType, targetValue, bufferLength, strLenOrInd);
}

SQLRETURN SQL_API SQLBindCol(SQLHSTMT statementHandle,
                             SQLUSMALLINT columnNumber,
                             SQLSMALLINT targetType,
                             SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLenOrInd) {
    auto stmt = static_cast<NYdb::NOdbc::TStatement*>(statementHandle);
    if (!stmt) {
        return SQL_INVALID_HANDLE;
    }
    return stmt->BindCol(columnNumber, targetType, targetValue, bufferLength, strLenOrInd);
}

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT handleType,
                                SQLHANDLE handle,
                                SQLSMALLINT recNumber,
                                SQLCHAR* sqlState,
                                SQLINTEGER* nativeError,
                                SQLCHAR* messageText,
                                SQLSMALLINT bufferLength,
                                SQLSMALLINT* textLength) {
    if (!handle) {
        return SQL_INVALID_HANDLE;
    }
    
    try {
        switch (handleType) {
            case SQL_HANDLE_ENV: {
                auto env = static_cast<NYdb::NOdbc::TEnvironment*>(handle);
                return env->GetDiagRec(recNumber, sqlState, nativeError, messageText, bufferLength, textLength);
            }
            
            case SQL_HANDLE_DBC: {
                auto conn = static_cast<NYdb::NOdbc::TConnection*>(handle);
                return conn->GetDiagRec(recNumber, sqlState, nativeError, messageText, bufferLength, textLength);
            }
            
            case SQL_HANDLE_STMT: {
                auto stmt = static_cast<NYdb::NOdbc::TStatement*>(handle);
                return stmt->GetDiagRec(recNumber, sqlState, nativeError, messageText, bufferLength, textLength);
            }
            
            default:
                return SQL_ERROR;
        }
    } catch (...) {
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
    auto stmt = static_cast<NYdb::NOdbc::TStatement*>(statementHandle);
    if (!stmt) {
        return SQL_INVALID_HANDLE;
    }
    
    return stmt->BindParameter(paramNumber, inputOutputType, valueType, parameterType, columnSize, decimalDigits, parameterValuePtr, bufferLength, strLenOrIndPtr);
}

SQLRETURN SQL_API SQLEndTran(SQLSMALLINT handleType, SQLHANDLE handle, SQLSMALLINT completionType) {
    if (!handle) {
        return SQL_INVALID_HANDLE;
    }
    try {
        switch (handleType) {
            case SQL_HANDLE_DBC: {
                auto conn = static_cast<NYdb::NOdbc::TConnection*>(handle);
                if (completionType == SQL_COMMIT) {
                    return conn->CommitTx();
                } else if (completionType == SQL_ROLLBACK) {
                    return conn->RollbackTx();
                } else {
                    return SQL_ERROR;
                }
            }
            case SQL_HANDLE_STMT: {
                auto stmt = static_cast<NYdb::NOdbc::TStatement*>(handle);
                auto conn = stmt->GetConnection();
                if (!conn) return SQL_INVALID_HANDLE;
                if (completionType == SQL_COMMIT) {
                    return conn->CommitTx();
                } else if (completionType == SQL_ROLLBACK) {
                    return conn->RollbackTx();
                } else {
                    return SQL_ERROR;
                }
            }
            case SQL_HANDLE_ENV: {
                // TODO: if's list of connections in ENV, go through them and commit/rollback transactions
                return SQL_SUCCESS;
            }
            default:
                return SQL_ERROR;
        }
    } catch (...) {
        return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC connectionHandle, SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength) {
    auto conn = static_cast<NYdb::NOdbc::TConnection*>(connectionHandle);
    if (!conn) {
        return SQL_INVALID_HANDLE;
    }
    if (attribute == SQL_ATTR_AUTOCOMMIT) {
        if ((intptr_t)value == SQL_AUTOCOMMIT_ON) {
            return conn->SetAutocommit(true);
        } else if ((intptr_t)value == SQL_AUTOCOMMIT_OFF) {
            return conn->SetAutocommit(false);
        } else {
            return SQL_ERROR;
        }
    }
    // TODO: other attributes
    return SQL_ERROR;
}

SQLRETURN SQL_API SQLColumns(SQLHSTMT statementHandle,
                             SQLCHAR* catalogName, SQLSMALLINT nameLength1,
                             SQLCHAR* schemaName, SQLSMALLINT nameLength2,
                             SQLCHAR* tableName, SQLSMALLINT nameLength3,
                             SQLCHAR* columnName, SQLSMALLINT nameLength4) {
    auto stmt = static_cast<NYdb::NOdbc::TStatement*>(statementHandle);
    if (!stmt) {
        return SQL_INVALID_HANDLE;
    }
    return stmt->Columns(
        NYdb::NOdbc::GetString(catalogName, nameLength1),
        NYdb::NOdbc::GetString(schemaName, nameLength2),
        NYdb::NOdbc::GetString(tableName, nameLength3),
        NYdb::NOdbc::GetString(columnName, nameLength4));
}

SQLRETURN SQL_API SQLTables(SQLHSTMT statementHandle,
                             SQLCHAR* catalogName, SQLSMALLINT nameLength1,
                             SQLCHAR* schemaName, SQLSMALLINT nameLength2,
                             SQLCHAR* tableName, SQLSMALLINT nameLength3,
                             SQLCHAR* tableType, SQLSMALLINT nameLength4) {
    auto stmt = static_cast<NYdb::NOdbc::TStatement*>(statementHandle);
    if (!stmt) {
        return SQL_INVALID_HANDLE;
    }
    return stmt->Tables(
        NYdb::NOdbc::GetString(catalogName, nameLength1),
        NYdb::NOdbc::GetString(schemaName, nameLength2),
        NYdb::NOdbc::GetString(tableName, nameLength3),
        NYdb::NOdbc::GetString(tableType, nameLength4));
}

}
