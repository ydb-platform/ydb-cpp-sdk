#pragma once

#include <string>

namespace NYdb::NOdbc {

std::string RewriteOdbcEscapes(const std::string& sql);

} // namespace NYdb::NOdbc
