#pragma once

#include "utils/error_manager.h"

#include <ydb-cpp-sdk/client/query/tx.h>

#include <functional>
#include <optional>

#include <sql.h>
#include <sqlext.h>

namespace NYdb {
namespace NOdbc {

class TConnectionAttributes {
public:
    SQLRETURN SetAutocommit(bool value);
    bool GetAutocommit() const;

    SQLRETURN SetConnectAttr(
        SQLINTEGER attr,
        SQLPOINTER value,
        SQLINTEGER stringLength,
        const std::function<SQLRETURN(bool)>& applyAutocommit,
        TErrorManager& errors);

    SQLRETURN GetConnectAttr(
        SQLINTEGER attr,
        SQLPOINTER value,
        SQLINTEGER bufferLength,
        SQLINTEGER* stringLengthPtr,
        TErrorManager& errors) const;

    NQuery::TTxSettings MakeTxSettings() const;

private:
    static std::optional<NQuery::TTxSettings::ETransactionMode> ResolveTxMode(SQLUINTEGER accessMode, SQLUINTEGER txnIsolation);

private:
    bool Autocommit_ = true;
    SQLUINTEGER AccessMode_ = SQL_MODE_READ_WRITE;
    SQLUINTEGER TxnIsolation_ = SQL_TXN_SERIALIZABLE;
    NQuery::TTxSettings::ETransactionMode TxMode_ = NQuery::TTxSettings::TS_SERIALIZABLE_RW;
};

} // namespace NOdbc
} // namespace NYdb
