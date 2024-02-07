#pragma once

#include <client/impl/ydb_internal/common/type_switcher.h>


namespace NYdb {

// C++17 support for external users
bool StringStartsWith(const std::string& line, const std::string& pattern);
std::string ToStringType(const std::string& str);

} // namespace NYdb

