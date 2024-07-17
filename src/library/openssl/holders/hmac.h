#pragma once

#include "holder.h"

#include <openssl/hmac.h>

namespace NOpenSSL {
    class THmacCtx : public std::unique_ptr<HMAC_CTX, HMAC_CTX_new, HMAC_CTX_free> {
    };
}
