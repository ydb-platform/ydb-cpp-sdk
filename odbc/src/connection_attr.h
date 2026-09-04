#pragma once

#include "utils/attr.h"

#include <ydb-cpp-sdk/client/query/tx.h>

#include <optional>
#include <string>

namespace NYdb::NOdbc {

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

    SQLRETURN SetAutocommit(bool value) {
        Autocommit_ = value;
        return SQL_SUCCESS;
    }
    bool GetAutocommit() const { return Autocommit_; }

    SQLRETURN SetConnectAttr(
        SQLINTEGER attr,
        SQLPOINTER value,
        TErrorManager& errors);

    SQLRETURN GetConnectAttr(
        SQLINTEGER attr,
        SQLPOINTER value,
        SQLINTEGER bufferLength,
        SQLINTEGER* stringLengthPtr,
        TErrorManager& errors) const;

    NQuery::TTxSettings MakeTxSettings() const;
    void SetCurrentCatalog(const std::string& value) {
        CurrentCatalog_ = value;
        NormalizeCatalogPath(CurrentCatalog_);
    }
    const std::string& GetCurrentCatalog() const { return CurrentCatalog_; }
    TCatalogBinding BuildCatalogBinding(const std::string& database) const;
    TCatalogRoute ResolveCatalogRoute(const std::string& currentDatabase) const;
    SQLRETURN ApplyCatalogChange(
        SQLPOINTER value,
        SQLINTEGER stringLength,
        const std::string& currentDatabase,
        std::optional<std::string>& rebindDatabase,
        TErrorManager& errors);
    static void NormalizeCatalogPath(std::string& path);
    SQLUINTEGER GetSupportedTxnIsolationOptions() const;
    SQLUINTEGER GetAccessMode() const { return AccessMode_; }

private:
    SQLRETURN SetAccessMode(SQLPOINTER value, TErrorManager& errors);
    SQLRETURN SetTxnIsolation(SQLPOINTER value, TErrorManager& errors);
    SQLRETURN SetCurrentCatalog(SQLPOINTER value, SQLINTEGER stringLength, TErrorManager& errors);

    bool Autocommit_ = true;
    std::string CurrentCatalog_;
    std::optional<SQLPOINTER> QuietMode_;
    std::optional<SQLUINTEGER> TranslateOption_;
    SQLUINTEGER LoginTimeout_ = 0;
    SQLUINTEGER AccessMode_ = SQL_MODE_READ_WRITE;
    SQLUINTEGER TxnIsolation_ = SQL_TXN_SERIALIZABLE;
    using TStoredProperties = TScalarProperties<
        TScalarProperty<SQL_ATTR_LOGIN_TIMEOUT, &TConnectionAttributes::LoginTimeout_>,
        TScalarProperty<SQL_ATTR_QUIET_MODE, &TConnectionAttributes::QuietMode_>,
        TScalarProperty<SQL_ATTR_TRANSLATE_OPTION, &TConnectionAttributes::TranslateOption_>>;
    using TReadOnlyProperties = TScalarProperties<
        TScalarProperty<SQL_ATTR_ACCESS_MODE, &TConnectionAttributes::AccessMode_>,
        TScalarProperty<SQL_ATTR_TXN_ISOLATION, &TConnectionAttributes::TxnIsolation_>>;
};

} // namespace NYdb::NOdbc
