#include "statement_attr.h"

#include "utils/attr.h"
#include "utils/diag.h"

#include <cstdint>

namespace NYdb {
namespace NOdbc {

SQLRETURN TStatementAttributes::SetStmtAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER /*stringLength*/,
    TErrorManager& errors) {
    switch (attr) {
        case SQL_ATTR_QUERY_TIMEOUT: {
            const SQLINTEGER timeout = ReadIntegerAttr<SQLINTEGER>(value);
            if (timeout < 0) {
                return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_QUERY_TIMEOUT");
            }
            QueryTimeoutSec_ = static_cast<SQLUINTEGER>(timeout);
            return SQL_SUCCESS;
        }
        case SQL_ATTR_MAX_ROWS: {
            const SQLLEN maxRows = ReadIntegerAttr<SQLLEN>(value);
            if (maxRows < 0) {
                return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_MAX_ROWS");
            }
            MaxRows_ = static_cast<SQLULEN>(maxRows);
            return SQL_SUCCESS;
        }
        case SQL_ATTR_NOSCAN: {
            const auto mode = ReadIntegerAttrIfIn<SQLULEN>(value, {SQL_NOSCAN_OFF, SQL_NOSCAN_ON});
            if (!mode) {
                return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_NOSCAN");
            }
            NoScan_ = *mode;
            return SQL_SUCCESS;
        }
        case SQL_ATTR_METADATA_ID: {
            const auto mode = ReadIntegerAttrIfIn<SQLULEN>(value, {SQL_FALSE, SQL_TRUE});
            if (!mode) {
                return Diag::AddInvalidAttrValue(errors, "SQL_ATTR_METADATA_ID");
            }
            MetadataId_ = *mode;
            return SQL_SUCCESS;
        }
        default:
            return Diag::AddNotImplemented(errors);
    }
}

SQLRETURN TStatementAttributes::GetStmtAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER /*bufferLength*/,
    SQLINTEGER* stringLengthPtr,
    TErrorManager& errors) const {
    if (!value) {
        return Diag::AddNullPointer(errors);
    }
    if (stringLengthPtr) {
        *stringLengthPtr = 0;
    }
    switch (attr) {
        case SQL_ATTR_QUERY_TIMEOUT:
            *reinterpret_cast<SQLUINTEGER*>(value) = QueryTimeoutSec_;
            return SQL_SUCCESS;
        case SQL_ATTR_MAX_ROWS:
            *reinterpret_cast<SQLULEN*>(value) = MaxRows_;
            return SQL_SUCCESS;
        case SQL_ATTR_NOSCAN:
            *reinterpret_cast<SQLULEN*>(value) = NoScan_;
            return SQL_SUCCESS;
        case SQL_ATTR_METADATA_ID:
            *reinterpret_cast<SQLULEN*>(value) = MetadataId_;
            return SQL_SUCCESS;
        default:
            return Diag::AddNotImplemented(errors);
    }
}

SQLUINTEGER TStatementAttributes::GetQueryTimeoutSec() const noexcept{
    return QueryTimeoutSec_;
}

SQLULEN TStatementAttributes::GetMaxRows() const noexcept {
    return MaxRows_;
}

SQLULEN TStatementAttributes::GetNoScanMode() const noexcept {
    return NoScan_;
}

SQLULEN TStatementAttributes::GetMetadataId() const noexcept {
    return MetadataId_;
}

} // namespace NOdbc
} // namespace NYdb
