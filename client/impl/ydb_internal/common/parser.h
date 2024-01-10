#pragma once

#include <client/impl/ydb_internal/common/type_switcher.h>

namespace NYdb {

struct TConnectionInfo {
    TStringType Endpoint = "";
    TStringType Database = "";
    bool EnableSsl = false;
};

TConnectionInfo ParseConnectionString(const TStringType& connectionString);

} // namespace NYdb

