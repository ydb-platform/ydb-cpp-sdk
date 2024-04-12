//
// Created by Evgeny Sidorov on 12/04/17.
//

#include "proxy_ref.h"
#include <src/library/digest/argonish/internal/argon2/argon2_base.h>
#include <src/library/digest/argonish/internal/argon2/argon2_ref.h>
#include <src/library/digest/argonish/internal/blake2b/blake2b.h>
#include <src/library/digest/argonish/internal/blake2b/blake2b_ref.h>

#include <stdexcept>

#define ZEROUPPER ;

namespace NArgonish {
    ARGON2_PROXY_CLASS_IMPL(REF)
    BLAKE2B_PROXY_CLASS_IMPL(REF)
}

#undef ZEROUPPER
