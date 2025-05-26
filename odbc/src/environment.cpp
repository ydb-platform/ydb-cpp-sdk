#include "environment.h"
#include "connection.h"

namespace NYdb {
namespace NOdbc {

TEnvironment::TEnvironment() : OdbcVersion_(SQL_OV_ODBC3) {}
TEnvironment::~TEnvironment() {}

SQLRETURN TEnvironment::SetAttribute(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength) {
    // TODO: реализовать обработку атрибутов
    OdbcVersion_ = attribute == SQL_ATTR_ODBC_VERSION ? reinterpret_cast<intptr_t>(value) : 0;
    return SQL_SUCCESS;
}

SQLRETURN TEnvironment::GetDiagRec(SQLSMALLINT recNumber,
                                   SQLCHAR* sqlState,
                                   SQLINTEGER* nativeError,
                                   SQLCHAR* messageText,
                                   SQLSMALLINT bufferLength,
                                   SQLSMALLINT* textLength) {

    if (recNumber < 1 || recNumber > (SQLSMALLINT)Errors_.size()) {
        return SQL_NO_DATA;
    }

    const auto& err = Errors_[recNumber-1];
    if (sqlState) {
        strncpy((char*)sqlState, err.SqlState.c_str(), 6);
    }

    if (nativeError) {
        *nativeError = err.NativeError;
    }

    if (messageText && bufferLength > 0) {
        strncpy((char*)messageText, err.Message.c_str(), bufferLength);
        if (textLength) {
            *textLength = (SQLSMALLINT)std::min((int)err.Message.size(), (int)bufferLength);
        }
    }
    return SQL_SUCCESS;
}

void TEnvironment::AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message) {
    Errors_.push_back({sqlState, nativeError, message});
}

void TEnvironment::ClearErrors() {
    Errors_.clear();
}

} // namespace NOdbc
} // namespace NYdb
