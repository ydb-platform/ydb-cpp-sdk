#pragma once

#include <client/impl/ydb_internal/common/type_switcher.h>

namespace NYdb {

struct TConnectionInfo {
    std::string Endpoint = "";
    std::string Database = "";
    bool EnableSsl = false;
};

TConnectionInfo ParseConnectionString(const std::string& connectionString);

} // namespace NYdb

