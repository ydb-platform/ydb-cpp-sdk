#pragma once

#include <src/util/generic/yexception.h>
#include <src/library/digest/argonish/argon2.h>
#include <src/library/digest/argonish/blake2b.h>
#include <src/library/digest/argonish/internal/proxies/macro/proxy_macros.h>

namespace NArgonish {
    ARGON2_PROXY_CLASS_DECL(AVX2)
    BLAKE2B_PROXY_CLASS_DECL(AVX2)
}
