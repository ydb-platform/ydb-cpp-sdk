#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
=======
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/defaults.h>
>>>>>>> origin/main

namespace NArgonish {
    /**
     * Instruction sets for which Argon2 is optimized
     */
    enum class EInstructionSet : ui32 {
        REF = 0,   /// Reference implementation
#if !defined(_arm64_)
        SSE2 = 1,  /// SSE2 optimized version
        SSSE3 = 2, /// SSSE3 optimized version
        SSE41 = 3, /// SSE4.1 optimized version
        AVX2 = 4   /// AVX2 optimized version
#endif
    };
}
