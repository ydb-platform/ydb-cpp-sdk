#include "unescape.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/escape.h>
=======
#include <src/util/string/escape.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

std::string_view UnescapeJsonUnicode(std::string_view data, char* scratch) {
    return std::string_view(scratch, UnescapeC(data.data(), data.size(), scratch));
}
