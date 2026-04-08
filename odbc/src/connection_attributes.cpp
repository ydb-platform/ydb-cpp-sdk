#include "connection_attributes.h"

#include <cstdint>

namespace NYdb {
namespace NOdbc {

std::optional<NQuery::TTxSettings::ETransactionMode> TConnectionAttributes::ResolveTxMode(SQLUINTEGER accessMode, SQLUINTEGER txnIsolation) {
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
    SQLINTEGER /*stringLength*/,
    const std::function<SQLRETURN(bool)>& applyAutocommit,
    TErrorManager& errors) {
    switch (attr) {
        case SQL_ATTR_AUTOCOMMIT: {
            const intptr_t val = reinterpret_cast<intptr_t>(value);
            if (val == static_cast<intptr_t>(SQL_AUTOCOMMIT_ON)) {
                return applyAutocommit(true);
            }
            if (val == static_cast<intptr_t>(SQL_AUTOCOMMIT_OFF)) {
                return applyAutocommit(false);
            }
            return errors.AddError("HY024", 0, "Invalid SQL_ATTR_AUTOCOMMIT value");
        }
        case SQL_ATTR_ACCESS_MODE: {
            const intptr_t val = reinterpret_cast<intptr_t>(value);
            if (val == static_cast<intptr_t>(SQL_MODE_READ_WRITE)) {
                AccessMode_ = SQL_MODE_READ_WRITE;
                auto txMode = ResolveTxMode(AccessMode_, TxnIsolation_);
                if (!txMode) {
                    return errors.AddError("HYC00", 0, "Transaction isolation is not supported for read-write mode");
                }
                TxMode_ = *txMode;
                return SQL_SUCCESS;
            }
            if (val == static_cast<intptr_t>(SQL_MODE_READ_ONLY)) {
                AccessMode_ = SQL_MODE_READ_ONLY;
                auto txMode = ResolveTxMode(AccessMode_, TxnIsolation_);
                if (!txMode) {
                    return errors.AddError("HYC00", 0, "Transaction isolation is not supported for read-only mode");
                }
                TxMode_ = *txMode;
                return SQL_SUCCESS;
            }
            return errors.AddError("HY024", 0, "Invalid SQL_ATTR_ACCESS_MODE value");
        }
        case SQL_ATTR_TXN_ISOLATION: {
            const intptr_t val = reinterpret_cast<intptr_t>(value);
            const SQLUINTEGER isolation = static_cast<SQLUINTEGER>(val);
            auto txMode = ResolveTxMode(AccessMode_, isolation);
            if (!txMode) {
                return errors.AddError("HYC00", 0, "SQL_ATTR_TXN_ISOLATION value is not supported");
            }
            TxnIsolation_ = isolation;
            TxMode_ = *txMode;
            return SQL_SUCCESS;
        }
        default:
            return errors.AddError("HYC00", 0, "Optional feature not implemented");
    }
}

SQLRETURN TConnectionAttributes::GetConnectAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER /*bufferLength*/,
    SQLINTEGER* stringLengthPtr,
    TErrorManager& errors) const {
    if (!value) {
        return errors.AddError("HY009", 0, "Invalid use of null pointer");
    }
    if (stringLengthPtr) {
        *stringLengthPtr = 0;
    }
    auto* out = reinterpret_cast<SQLUINTEGER*>(value);
    switch (attr) {
        case SQL_ATTR_AUTOCOMMIT:
            *out = GetAutocommit() ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;
            return SQL_SUCCESS;
        case SQL_ATTR_ACCESS_MODE:
            *out = AccessMode_;
            return SQL_SUCCESS;
        case SQL_ATTR_TXN_ISOLATION:
            *out = TxnIsolation_;
            return SQL_SUCCESS;
        default:
            return errors.AddError("HYC00", 0, "Optional feature not implemented");
    }
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

} // namespace NOdbc
} // namespace NYdb
