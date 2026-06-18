#pragma once

#include "utils/error_manager.h"

#include <sql.h>
#include <sqlext.h>

namespace NYdb {
namespace NOdbc {

class TStatementAttributes {
public:
    SQLRETURN SetStmtAttr(
        SQLINTEGER attr,
        SQLPOINTER value,
        SQLINTEGER stringLength,
        TErrorManager& errors);

    SQLRETURN GetStmtAttr(
        SQLINTEGER attr,
        SQLPOINTER value,
        SQLINTEGER bufferLength,
        SQLINTEGER* stringLengthPtr,
        TErrorManager& errors) const;

    SQLUINTEGER GetQueryTimeoutSec() const noexcept;
    SQLULEN GetMaxRows() const noexcept;
    SQLULEN GetNoScanMode() const noexcept;
    SQLULEN GetMetadataId() const noexcept;

private:
    SQLUINTEGER QueryTimeoutSec_ = 0;
    SQLULEN MaxRows_ = 0;
    SQLULEN NoScan_ = SQL_NOSCAN_OFF;
    SQLULEN MetadataId_ = SQL_FALSE;
};

} // namespace NOdbc
} // namespace NYdb
