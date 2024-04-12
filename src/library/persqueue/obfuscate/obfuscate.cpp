#include "obfuscate.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/utility.h>
=======
#include <src/util/generic/utility.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NPersQueue {

std::string ObfuscateString(std::string str) {
    ui32 publicPartSize = Min<ui32>(4, str.size() / 4);
    for (ui32 i = publicPartSize; i < str.size() - publicPartSize; ++i) {
        str[i] = '*';
    }
    return str;
}

} // namespace NPersQueue
