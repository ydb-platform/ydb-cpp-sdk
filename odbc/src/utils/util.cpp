#include "util.h"

namespace NYdb::NOdbc {

std::string GetString(SQLCHAR* str, SQLSMALLINT length) {
    if (!str) {
        return {};
    }
    if (length == SQL_NTS) {
        return std::string(reinterpret_cast<const char*>(str));
    }
    if (length <= 0) {
        return {};
    }
    return std::string(reinterpret_cast<const char*>(str), length);
}

} // namespace NYdb::NOdbc
