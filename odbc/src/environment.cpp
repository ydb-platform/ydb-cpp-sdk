#include "environment.h"
#include "connection.h"

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

void TEnvironment::RegisterConnection(TConnection* conn){
    if (conn == nullptr){
        throw std::invalid_argument("null connection");
    }
    connections_.insert(conn);
}

void TEnvironment::UnregisterConnection(TConnection* conn){
    if (conn == nullptr){
        throw std::invalid_argument("null connection");
    }
    connections_.erase(conn);
}

std::vector<TConnection*> TEnvironment::GetConnectionsSnapshot() const {
    return std::vector<TConnection*>(connections_.begin(), connections_.end());
}

SQLRETURN TEnvironment::EndTran(SQLSMALLINT completionType){
    if (completionType != SQL_COMMIT && completionType != SQL_ROLLBACK){
        return AddError("HY012", 0, "Invalid transaction operation code");
    }
    bool hasFailures = false;
    int failedCount = 0;
    
    for (auto* conn : connections_) {
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
        AddError("01000", 0, "SQLEndTran(SQL_HANDLE_ENV): some connections failed", SQL_SUCCESS_WITH_INFO);
        return SQL_SUCCESS_WITH_INFO;
    }
    return SQL_SUCCESS;
}
} // namespace NOdbc
} // namespace NYdb
