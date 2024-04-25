#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main
#include <src/library/digest/argonish/argon2.h>
#include <src/library/digest/argonish/blake2b.h>
#include <src/library/digest/argonish/internal/proxies/macro/proxy_macros.h>

namespace NArgonish {
    ARGON2_PROXY_CLASS_DECL(AVX2)
    BLAKE2B_PROXY_CLASS_DECL(AVX2)
}
