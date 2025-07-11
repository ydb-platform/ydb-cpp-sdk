#include "environment.h"
#include "connection.h"
#include "statement.h"

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
            return NYdb::NOdbc::HandleOdbcExceptions(inputHandle, [&]() {
                *outputHandle = new NYdb::NOdbc::TEnvironment();
                return SQL_SUCCESS;
            });
        }

        case SQL_HANDLE_DBC: {
            return NYdb::NOdbc::HandleOdbcExceptions(inputHandle, [&]() {
                *outputHandle = new NYdb::NOdbc::TConnection();
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_STMT: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(inputHandle, [&](auto* conn) {
                auto stmt = conn->CreateStatement();
                *outputHandle = stmt.release();
                return SQL_SUCCESS;
            });
        }
        default:
            return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT handleType, SQLHANDLE handle) {
    switch (handleType) {
        case SQL_HANDLE_ENV: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TEnvironment>(handle, [](auto* env) {
                delete env;
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_DBC: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(handle, [](auto* conn) {
                delete conn;
                return SQL_SUCCESS;
            });
        }
        case SQL_HANDLE_STMT: {
            return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(handle, [](auto* stmt) {
                if (stmt->GetConnection()) {
                    stmt->GetConnection()->RemoveStatement(stmt);
                }
                delete stmt;
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
            // TODO: if's list of connections in ENV, go through them and commit/rollback transactions
            return SQL_SUCCESS;
        }
        default:
            return SQL_ERROR;
    }
}

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC connectionHandle, SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength) {
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TConnection>(connectionHandle, [&](auto* conn) {
        if (attribute == SQL_ATTR_AUTOCOMMIT) {
            if ((intptr_t)value == SQL_AUTOCOMMIT_ON) {
                return conn->SetAutocommit(true);
            } else if ((intptr_t)value == SQL_AUTOCOMMIT_OFF) {
                return conn->SetAutocommit(false);
            } else {
                throw NYdb::NOdbc::TOdbcException("HY000", 0, "Invalid autocommit value");
            }
        }
        // TODO: other attributes
        throw NYdb::NOdbc::TOdbcException("HYC00", 0, "Optional feature not implemented");
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
    return NYdb::NOdbc::HandleOdbcExceptions<NYdb::NOdbc::TStatement>(statementHandle, [&](auto* stmt) -> SQLRETURN {
        switch (option) {
            case SQL_CLOSE:
                return stmt->Close(true);
            case SQL_DROP:
                return SQLFreeHandle(SQL_HANDLE_STMT, statementHandle);
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

}
