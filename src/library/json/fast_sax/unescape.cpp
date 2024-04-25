#include "unescape.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/escape.h>
=======
#include <src/util/string/escape.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/escape.h>
>>>>>>> origin/main

std::string_view UnescapeJsonUnicode(std::string_view data, char* scratch) {
    return std::string_view(scratch, UnescapeC(data.data(), data.size(), scratch));
}
