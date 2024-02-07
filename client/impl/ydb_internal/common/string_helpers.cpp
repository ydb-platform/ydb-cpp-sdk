#include "string_helpers.h"

#include <algorithm>


namespace NYdb {

bool StringStartsWith(const std::string& line, const std::string& pattern) {
    return std::equal(pattern.begin(), pattern.end(), line.begin());
}

std::string ToStringType(const std::string& str) {
    return std::string(str.data(), str.size());
}

} // namespace NYdb
