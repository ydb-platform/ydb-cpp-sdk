#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
#include <src/library/digest/argonish/argon2.h>
#include <src/library/digest/argonish/blake2b.h>
#include <src/library/digest/argonish/internal/proxies/macro/proxy_macros.h>

namespace NArgonish {
    ARGON2_PROXY_CLASS_DECL(SSE2)
    BLAKE2B_PROXY_CLASS_DECL(SSE2)
}
