#pragma once

#include "holder.h"

#include <openssl/evp.h>

namespace NOpenSSL {
    class TEvpCipherCtx : public std::unique_ptr<EVP_CIPHER_CTX, EVP_CIPHER_CTX_new, EVP_CIPHER_CTX_free> {
    };

    class TEvpMdCtx : public std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_new, EVP_MD_CTX_free> {
    };
}
