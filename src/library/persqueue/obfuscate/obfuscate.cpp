#include "obfuscate.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/utility.h>
=======
#include <src/util/generic/utility.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/utility.h>
>>>>>>> origin/main

namespace NPersQueue {

std::string ObfuscateString(std::string str) {
    ui32 publicPartSize = Min<ui32>(4, str.size() / 4);
    for (ui32 i = publicPartSize; i < str.size() - publicPartSize; ++i) {
        str[i] = '*';
    }
    return str;
}

} // namespace NPersQueue
