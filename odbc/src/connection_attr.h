#pragma once

#include "utils/error_manager.h"

#include <ydb-cpp-sdk/client/query/tx.h>

#include <functional>
#include <optional>
#include <string>

#include <sql.h>
#include <sqlext.h>

namespace NYdb {
namespace NOdbc {

class TConnectionAttributes {
public:
    struct TCatalogBinding {
        std::string Catalog;
        std::string Database;
        std::optional<std::string> RelativeCatalog;
    };

    struct TCatalogRoute {
        std::string EffectiveDatabase;
        std::optional<std::string> TablePathPrefix;
    };

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
    void SetCurrentCatalog(const std::string& value);
    const std::string& GetCurrentCatalog() const;
    TCatalogBinding BuildCatalogBinding(const std::string& database) const;
    TCatalogRoute ResolveCatalogRoute(const std::string& currentDatabase) const;
    SQLRETURN ApplyCatalogChange(
        SQLPOINTER value,
        SQLINTEGER stringLength,
        const std::string& currentDatabase,
        std::optional<std::string>& rebindDatabase,
        TErrorManager& errors);
    static void NormalizeCatalogPath(std::string& path);

private:
    SQLRETURN SetAutocommit(
        SQLPOINTER value,
        const std::function<SQLRETURN(bool)>& applyAutocommit,
        TErrorManager& errors);
    SQLRETURN SetAccessMode(SQLPOINTER value, TErrorManager& errors);
    SQLRETURN SetTxnIsolation(SQLPOINTER value, TErrorManager& errors);
    SQLRETURN SetCurrentCatalog(SQLPOINTER value, SQLINTEGER stringLength, TErrorManager& errors);

    SQLRETURN GetAutocommit(SQLPOINTER value) const;
    SQLRETURN GetAccessMode(SQLPOINTER value) const;
    SQLRETURN GetTxnIsolation(SQLPOINTER value) const;
    SQLRETURN GetCurrentCatalog(
        SQLPOINTER value,
        SQLINTEGER bufferLength,
        SQLINTEGER* stringLengthPtr,
        TErrorManager& errors) const;

    bool Autocommit_ = true;
    std::string CurrentCatalog_;
    SQLUINTEGER AccessMode_ = SQL_MODE_READ_WRITE;
    SQLUINTEGER TxnIsolation_ = SQL_TXN_SERIALIZABLE;
    NQuery::TTxSettings::ETransactionMode TxMode_ = NQuery::TTxSettings::TS_SERIALIZABLE_RW;
};

} // namespace NOdbc
} // namespace NYdb
