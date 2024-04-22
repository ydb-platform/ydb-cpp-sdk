#include "unescape.h"

#include <ydb-cpp-sdk/util/string/escape.h>

std::string_view UnescapeJsonUnicode(std::string_view data, char* scratch) {
    return std::string_view(scratch, UnescapeC(data.data(), data.size(), scratch));
}
