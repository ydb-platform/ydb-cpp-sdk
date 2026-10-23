#include "connection_attr.h"

namespace NYdb::NOdbc {

namespace {

namespace Tx {

bool IsKnownTxnIsolation(SQLUINTEGER txnIsolation) {
    return txnIsolation == SQL_TXN_READ_UNCOMMITTED
        || txnIsolation == SQL_TXN_READ_COMMITTED
        || txnIsolation == SQL_TXN_REPEATABLE_READ
        || txnIsolation == SQL_TXN_SERIALIZABLE;
}

std::optional<NQuery::TTxSettings::ETransactionMode> ResolveTxMode(SQLUINTEGER accessMode, SQLUINTEGER txnIsolation) {
    if (accessMode == SQL_MODE_READ_ONLY) {
        return NQuery::TTxSettings::TS_SNAPSHOT_RO;
    }

    switch (txnIsolation) {
        case SQL_TXN_REPEATABLE_READ:
            return NQuery::TTxSettings::TS_SNAPSHOT_RW;
        case SQL_TXN_SERIALIZABLE:
            return NQuery::TTxSettings::TS_SERIALIZABLE_RW;
        default:
            return std::nullopt;
    }
}

} // namespace Tx

} // namespace

void TConnectionAttributes::NormalizeCatalogPath(std::string& path) {
    if (path.empty() || path == "/") {
        return;
    }
    const size_t end = path.find_last_not_of('/');
    if (end == std::string::npos) {
        path = "/";
    } else {
        path.erase(end + 1);
    }
}

SQLRETURN TConnectionAttributes::SetConnectAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    TErrorManager& errors) {
    switch (attr) {
        case SQL_ATTR_ACCESS_MODE:
            return SetAccessMode(value, errors);
        case SQL_ATTR_TXN_ISOLATION:
            return SetTxnIsolation(value, errors);
        case SQL_ATTR_TRANSLATE_LIB:
            if (!value) {
                return Diag::AddNullPointer(errors);
            }
            return errors.AddError(
                "HYC00", 0, "Translation libraries are not supported");
        default:
            return TStoredProperties::Set(attr, *this, value)
                ? SQL_SUCCESS
                : Diag::AddNotImplemented(errors);
    }
}

SQLRETURN TConnectionAttributes::GetConnectAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER bufferLength,
    SQLINTEGER* stringLengthPtr,
    TErrorManager& errors) const {
    const bool stringAttribute =
        attr == SQL_ATTR_CURRENT_CATALOG || attr == SQL_ATTR_TRANSLATE_LIB;
    if (!value && (!stringAttribute || !stringLengthPtr)) {
        return Diag::AddNullPointer(errors);
    }
    if (stringLengthPtr) {
        *stringLengthPtr = 0;
    }
    if (attr == SQL_ATTR_AUTOCOMMIT) {
        *static_cast<SQLUINTEGER*>(value) = Autocommit_ ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;
        return SQL_SUCCESS;
    }
    if (attr == SQL_ATTR_CURRENT_CATALOG) {
        return Diag::WriteString<Diag::EStringWriteMode::ConnectionAttribute>(
            &errors, CurrentCatalog_, value, bufferLength, stringLengthPtr);
    }
    if (attr == SQL_ATTR_TRANSLATE_LIB) {
        return SQL_NO_DATA;
    }
    if (auto result = TStoredProperties::Get(attr, *this, value, stringLengthPtr)) {
        return *result;
    }
    if (auto result = TReadOnlyProperties::Get(attr, *this, value)) {
        return *result;
    }
    return Diag::AddNotImplemented(errors);
}

SQLRETURN TConnectionAttributes::SetAccessMode(SQLPOINTER value, TErrorManager& errors) {
    const SQLUINTEGER mode = ReadIntegerAttr<SQLUINTEGER>(value);
    if (mode != SQL_MODE_READ_WRITE && mode != SQL_MODE_READ_ONLY) {
        return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_ACCESS_MODE");
    }
    auto txMode = Tx::ResolveTxMode(mode, TxnIsolation_);
    if (!txMode) {
        return errors.AddError(
            "HYC00",
            0,
            mode == SQL_MODE_READ_WRITE
                ? "Transaction isolation is not supported for read-write mode"
                : "Transaction isolation is not supported for read-only mode");
    }
    AccessMode_ = mode;
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
    return SQL_SUCCESS;
}

SQLRETURN TConnectionAttributes::SetCurrentCatalog(SQLPOINTER value, SQLINTEGER stringLength, TErrorManager& errors) {
    if (!value) {
        return Diag::AddNullPointer(errors);
    }
    std::string catalog = ReadAttributeString(value, stringLength);
    NormalizeCatalogPath(catalog);
    if (catalog.empty()) {
        return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_CURRENT_CATALOG");
    }
    CurrentCatalog_ = std::move(catalog);
    return SQL_SUCCESS;
}

SQLUINTEGER TConnectionAttributes::GetSupportedTxnIsolationOptions() const {
    return AccessMode_ == SQL_MODE_READ_ONLY
        ? SQL_TXN_READ_UNCOMMITTED | SQL_TXN_READ_COMMITTED
            | SQL_TXN_REPEATABLE_READ | SQL_TXN_SERIALIZABLE
        : SQL_TXN_REPEATABLE_READ | SQL_TXN_SERIALIZABLE;
}

NQuery::TTxSettings TConnectionAttributes::MakeTxSettings() const {
    const auto mode = Tx::ResolveTxMode(AccessMode_, TxnIsolation_);
    if (mode == NQuery::TTxSettings::TS_SNAPSHOT_RO) {
        return NQuery::TTxSettings::SnapshotRO();
    }
    if (mode == NQuery::TTxSettings::TS_SNAPSHOT_RW) {
        return NQuery::TTxSettings::SnapshotRW();
    }
    return NQuery::TTxSettings::SerializableRW();
}

TConnectionAttributes::TCatalogBinding TConnectionAttributes::BuildCatalogBinding(const std::string& database) const {
    TCatalogBinding binding{CurrentCatalog_, database, std::nullopt};
    NormalizeCatalogPath(binding.Catalog);
    NormalizeCatalogPath(binding.Database);
    const std::string prefix = binding.Database + "/";
    if (binding.Catalog.size() > prefix.size()
        && binding.Catalog.compare(0, prefix.size(), prefix) == 0) {
        binding.RelativeCatalog = binding.Catalog.substr(prefix.size());
    }
    return binding;
}

TConnectionAttributes::TCatalogRoute TConnectionAttributes::ResolveCatalogRoute(
    const std::string& currentDatabase) const {
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
    rebindDatabase = route.EffectiveDatabase == currentDatabase
        ? std::nullopt
        : std::optional<std::string>(route.EffectiveDatabase);
    return SQL_SUCCESS;
}

} // namespace NYdb::NOdbc
