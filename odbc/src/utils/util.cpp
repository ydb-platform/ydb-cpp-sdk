#include "util.h"

namespace NYdb::NOdbc {

std::string GetString(SQLCHAR* str, SQLSMALLINT length) {
    if (length == SQL_NTS) {
        return std::string(reinterpret_cast<const char*>(str));
    }
    return std::string(reinterpret_cast<const char*>(str), length);
}

} // namespace NYdb::NOdbc
