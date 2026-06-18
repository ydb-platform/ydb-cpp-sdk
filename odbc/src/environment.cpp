#include "environment.h"
#include "connection.h"

 #include <exception>
 #include <stdexcept>

namespace NYdb {
namespace NOdbc {

TEnvironment::TEnvironment() : OdbcVersion_(SQL_OV_ODBC3) {}
TEnvironment::~TEnvironment() {}

SQLRETURN TEnvironment::SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength) {
    switch (attribute) {
        case SQL_ATTR_ODBC_VERSION: {
            if (!value) {
                return AddError("HY009", 0, "Invalid use of null pointer");
            }
            OdbcVersion_ = static_cast<SQLINTEGER>(reinterpret_cast<intptr_t>(value));
            return SQL_SUCCESS;
        }
        case SQL_ATTR_OUTPUT_NTS: {
            if (value && static_cast<SQLINTEGER>(reinterpret_cast<intptr_t>(value)) != SQL_TRUE) {
                return AddError("HY024", 0, "SQL_ATTR_OUTPUT_NTS must be SQL_TRUE");
            }
            return SQL_SUCCESS;
        }
        default:
            return AddError("HYC00", 0, "Optional feature not implemented");
    }
}

SQLRETURN TEnvironment::GetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr) {
    if (!value) {
        return AddError("HY009", 0, "Invalid use of null pointer");
    }
    if (stringLengthPtr) {
        *stringLengthPtr = 0;
    }
    switch (attribute) {
        case SQL_ATTR_ODBC_VERSION:
            if (bufferLength < static_cast<SQLINTEGER>(sizeof(SQLINTEGER))) {
                return AddError("HY090", 0, "Invalid string or buffer length");
            }
            *reinterpret_cast<SQLINTEGER*>(value) = OdbcVersion_;
            if (stringLengthPtr) {
                *stringLengthPtr = sizeof(SQLINTEGER);
            }
            return SQL_SUCCESS;
        case SQL_ATTR_OUTPUT_NTS:
            if (bufferLength < static_cast<SQLINTEGER>(sizeof(SQLINTEGER))) {
                return AddError("HY090", 0, "Invalid string or buffer length");
            }
            *reinterpret_cast<SQLINTEGER*>(value) = SQL_TRUE;
            if (stringLengthPtr) {
                *stringLengthPtr = sizeof(SQLINTEGER);
            }
            return SQL_SUCCESS;
        default:
            return AddError("HYC00", 0, "Optional feature not implemented");
    }
}

void TEnvironment::RegisterConnection(TConnection* conn){
    if (conn == nullptr){
        throw std::invalid_argument("null connection");
    }
    Connections_.insert(conn);
}

void TEnvironment::UnregisterConnection(TConnection* conn){
    if (conn == nullptr){
        throw std::invalid_argument("null connection");
    }
    Connections_.erase(conn);
}

std::vector<TConnection*> TEnvironment::GetConnectionsSnapshot() const {
    return std::vector<TConnection*>(Connections_.begin(), Connections_.end());
}

SQLRETURN TEnvironment::EndTran(SQLSMALLINT completionType){
    if (completionType != SQL_COMMIT && completionType != SQL_ROLLBACK){
        return AddError("HY012", 0, "Invalid transaction operation code");
    }
    bool hasFailures = false;
    int failedCount = 0;
    
    for (auto* conn : Connections_) {
        if (!conn || !conn->GetTx()) {
            continue;
        }
        try {
            if (completionType == SQL_COMMIT) {
                conn->CommitTx();
            } else {
                conn->RollbackTx();
            }
        } catch (const std::exception& ex) {
            hasFailures = true;
            ++failedCount;
            AddError("HY000", 0, ex.what(), SQL_SUCCESS_WITH_INFO);
        } catch (...) {
            hasFailures = true;
            ++failedCount;
            AddError("HY000", 0, "Unknown error during ENV-level transaction completion", SQL_SUCCESS_WITH_INFO);
        }
    }
    if (hasFailures) {
        AddError("01000", 0,
            "SQLEndTran(SQL_HANDLE_ENV): " + std::to_string(failedCount) + " connection(s) failed",
            SQL_SUCCESS_WITH_INFO);
        return SQL_SUCCESS_WITH_INFO;
    }
    return SQL_SUCCESS;
}
} // namespace NOdbc
} // namespace NYdb
