
#include "connection_attr.h"
#include "utils/attr.h"
#include "utils/diag.h"

#include <cstdint>

namespace NYdb {
namespace NOdbc {

namespace  {

namespace Catalog {

void NormalizePath(std::string& path) {
    if (path.empty() || path == "/") {
        return;
    }
    const size_t trailingSlashStart = path.find_last_not_of('/');
    if (trailingSlashStart == std::string::npos) {
        path.assign("/");
        return;
    }
    path.erase(trailingSlashStart + 1);
}

TConnectionAttributes::TCatalogBinding BuildBinding(const std::string& currentCatalog, const std::string& database) {
    TConnectionAttributes::TCatalogBinding binding;
    binding.Catalog = currentCatalog;
    binding.Database = database;
    NormalizePath(binding.Catalog);
    NormalizePath(binding.Database);
    if (binding.Catalog == binding.Database) {
        return binding;
    }

    const std::string databasePrefix = binding.Database + "/";
    if (binding.Catalog.size() <= databasePrefix.size() ||
        binding.Catalog.compare(0, databasePrefix.size(), databasePrefix) != 0) {
        return binding;
    }

    std::string relativeCatalog = binding.Catalog.substr(databasePrefix.size());
    if (!relativeCatalog.empty()) {
        binding.RelativeCatalog = std::move(relativeCatalog);
    }
    return binding;
}

} // namespace Catalog

namespace Tx {

bool IsKnownTxnIsolation(SQLUINTEGER txnIsolation) {
    switch (txnIsolation) {
        case SQL_TXN_READ_UNCOMMITTED:
        case SQL_TXN_READ_COMMITTED:
        case SQL_TXN_REPEATABLE_READ:
        case SQL_TXN_SERIALIZABLE:
            return true;
        default:
            return false;
    }
}

std::optional<NQuery::TTxSettings::ETransactionMode> ResolveTxMode(SQLUINTEGER accessMode, SQLUINTEGER txnIsolation) {
    if (accessMode == SQL_MODE_READ_ONLY) {
        switch (txnIsolation) {
            case SQL_TXN_READ_UNCOMMITTED:
                return NQuery::TTxSettings::TS_STALE_RO;
            case SQL_TXN_READ_COMMITTED:
                return NQuery::TTxSettings::TS_ONLINE_RO;
            case SQL_TXN_REPEATABLE_READ:
            case SQL_TXN_SERIALIZABLE:
                return NQuery::TTxSettings::TS_SNAPSHOT_RO;
            default:
                return std::nullopt;
        }
    }

    switch (txnIsolation) {
        case SQL_TXN_REPEATABLE_READ:
        case SQL_TXN_SERIALIZABLE:
            return NQuery::TTxSettings::TS_SERIALIZABLE_RW;
        default:
            return std::nullopt;
    }
}

} // namespace Tx

namespace Autocommit {

SQLRETURN Get(bool autocommitEnabled, SQLPOINTER value) {
    auto* out = reinterpret_cast<SQLUINTEGER*>(value);
    *out = autocommitEnabled ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;
    return SQL_SUCCESS;
}

} // namespace Autocommit

} 

void TConnectionAttributes::NormalizeCatalogPath(std::string& path) {
    Catalog::NormalizePath(path);
}

SQLRETURN TConnectionAttributes::SetAutocommit(bool value) {
    Autocommit_ = value;
    return SQL_SUCCESS;
}

bool TConnectionAttributes::GetAutocommit() const {
    return Autocommit_;
}

SQLRETURN TConnectionAttributes::SetConnectAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER stringLength,
    const std::function<SQLRETURN(bool)>& applyAutocommit,
    TErrorManager& errors) {
    switch (attr) {
        case SQL_ATTR_AUTOCOMMIT:
            return SetAutocommit(value, applyAutocommit, errors);
        case SQL_ATTR_ACCESS_MODE:
            return SetAccessMode(value, errors);
        case SQL_ATTR_TXN_ISOLATION:
            return SetTxnIsolation(value, errors);
        case SQL_ATTR_CURRENT_CATALOG:
            return SetCurrentCatalog(value, stringLength, errors);
        default:
            return Diag::AddNotImplemented(errors);
    }
}

SQLRETURN TConnectionAttributes::GetConnectAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER bufferLength,
    SQLINTEGER* stringLengthPtr,
    TErrorManager& errors) const {
    if (!value) {
        return Diag::AddNullPointer(errors);
    }
    if (stringLengthPtr) {
        *stringLengthPtr = 0;
    }
    switch (attr) {
        case SQL_ATTR_AUTOCOMMIT:
            return GetAutocommit(value);
        case SQL_ATTR_ACCESS_MODE:
            return GetAccessMode(value);
        case SQL_ATTR_TXN_ISOLATION:
            return GetTxnIsolation(value);
        case SQL_ATTR_CURRENT_CATALOG:
            return GetCurrentCatalog(value, bufferLength, stringLengthPtr, errors);
        default:
            return Diag::AddNotImplemented(errors);
    }
}

SQLRETURN TConnectionAttributes::SetAutocommit(
    SQLPOINTER value,
    const std::function<SQLRETURN(bool)>& applyAutocommit,
    TErrorManager& errors) {
    const auto token = ReadIntegerAttrIfIn<intptr_t>(
        value,
        {static_cast<intptr_t>(SQL_AUTOCOMMIT_ON), static_cast<intptr_t>(SQL_AUTOCOMMIT_OFF)});
    if (!token) {
        return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_AUTOCOMMIT");
    }
    if (*token == static_cast<intptr_t>(SQL_AUTOCOMMIT_ON)) {
        return applyAutocommit(true);
    }
    return applyAutocommit(false);
}

SQLRETURN TConnectionAttributes::SetAccessMode(SQLPOINTER value, TErrorManager& errors) {
    const auto mode = ReadIntegerAttrIfIn<SQLUINTEGER>(value, {SQL_MODE_READ_WRITE, SQL_MODE_READ_ONLY});
    if (!mode) {
        return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_ACCESS_MODE");
    }
    AccessMode_ = *mode;
    auto txMode = Tx::ResolveTxMode(AccessMode_, TxnIsolation_);
    if (!txMode) {
        return errors.AddError(
            "HYC00",
            0,
            AccessMode_ == SQL_MODE_READ_WRITE
                ? "Transaction isolation is not supported for read-write mode"
                : "Transaction isolation is not supported for read-only mode");
    }
    TxMode_ = *txMode;
    return SQL_SUCCESS;
}

SQLRETURN TConnectionAttributes::SetTxnIsolation(SQLPOINTER value, TErrorManager& errors) {
    const SQLUINTEGER isolation = ReadIntegerAttr<SQLUINTEGER>(value);
    if (!Tx::IsKnownTxnIsolation(isolation)) {
        return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_TXN_ISOLATION");
    }
    auto txMode = Tx::ResolveTxMode(AccessMode_, isolation);
    if (!txMode) {
        return errors.AddError("HYC00", 0, "SQL_ATTR_TXN_ISOLATION value is not supported");
    }
    TxnIsolation_ = isolation;
    TxMode_ = *txMode;
    return SQL_SUCCESS;
}

SQLRETURN TConnectionAttributes::SetCurrentCatalog(SQLPOINTER value, SQLINTEGER stringLength, TErrorManager& errors) {
    if (!value) {
        return Diag::AddNullPointer(errors);
    }
    CurrentCatalog_ = ReadAttributeString(value, stringLength);
    Catalog::NormalizePath(CurrentCatalog_);
    if (CurrentCatalog_.empty()) {
        return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_CURRENT_CATALOG");
    }
    return SQL_SUCCESS;
}

SQLRETURN TConnectionAttributes::GetAutocommit(SQLPOINTER value) const {
    return Autocommit::Get(Autocommit_, value);
}

SQLRETURN TConnectionAttributes::GetAccessMode(SQLPOINTER value) const {
    auto* out = reinterpret_cast<SQLUINTEGER*>(value);
    *out = AccessMode_;
    return SQL_SUCCESS;
}

SQLRETURN TConnectionAttributes::GetTxnIsolation(SQLPOINTER value) const {
    auto* out = reinterpret_cast<SQLUINTEGER*>(value);
    *out = TxnIsolation_;
    return SQL_SUCCESS;
}

SQLRETURN TConnectionAttributes::GetCurrentCatalog(
    SQLPOINTER value,
    SQLINTEGER bufferLength,
    SQLINTEGER* stringLengthPtr,
    TErrorManager& errors) const {
    return WriteAttributeString(CurrentCatalog_, value, bufferLength, stringLengthPtr, errors);
}

NQuery::TTxSettings TConnectionAttributes::MakeTxSettings() const {
    switch (TxMode_) {
        case NQuery::TTxSettings::TS_ONLINE_RO:
            return NQuery::TTxSettings::OnlineRO();
        case NQuery::TTxSettings::TS_STALE_RO:
            return NQuery::TTxSettings::StaleRO();
        case NQuery::TTxSettings::TS_SNAPSHOT_RO:
            return NQuery::TTxSettings::SnapshotRO();
        case NQuery::TTxSettings::TS_SNAPSHOT_RW:
            return NQuery::TTxSettings::SnapshotRW();
        case NQuery::TTxSettings::TS_SERIALIZABLE_RW:
        default:
            return NQuery::TTxSettings::SerializableRW();
    }
}

void TConnectionAttributes::SetCurrentCatalog(const std::string& value) {
    CurrentCatalog_ = value;
    Catalog::NormalizePath(CurrentCatalog_);
}

const std::string& TConnectionAttributes::GetCurrentCatalog() const {
    return CurrentCatalog_;
}

TConnectionAttributes::TCatalogBinding TConnectionAttributes::BuildCatalogBinding(const std::string& database) const {
    return Catalog::BuildBinding(CurrentCatalog_, database);
}

TConnectionAttributes::TCatalogRoute TConnectionAttributes::ResolveCatalogRoute(const std::string& currentDatabase) const {
    const TCatalogBinding binding = BuildCatalogBinding(currentDatabase);
    if (binding.Catalog == binding.Database) {
        return {binding.Database, std::nullopt};
    }
    if (binding.RelativeCatalog) {
        return {binding.Database, binding.Catalog};
    }
    return {binding.Catalog, std::nullopt};
}

SQLRETURN TConnectionAttributes::ApplyCatalogChange(
    SQLPOINTER value,
    SQLINTEGER stringLength,
    const std::string& currentDatabase,
    std::optional<std::string>& rebindDatabase,
    TErrorManager& errors) {
    SQLRETURN rc = SetCurrentCatalog(value, stringLength, errors);
    if (rc != SQL_SUCCESS) {
        return rc;
    }
    const TCatalogRoute route = ResolveCatalogRoute(currentDatabase);
    if (route.EffectiveDatabase != currentDatabase) {
        rebindDatabase = route.EffectiveDatabase;
    } else {
        rebindDatabase.reset();
    }
    return SQL_SUCCESS;
}

} // namespace NOdbc
} // namespace NYdb
