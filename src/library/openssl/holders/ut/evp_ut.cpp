#include "evp.h"

#include <src/library/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(Evp) {
    Y_UNIT_TEST(Cipher) {
        NOpenSSL::TEvpCipherCtx ctx;
        UNIT_ASSERT(ctx);
    }

    Y_UNIT_TEST(Md) {
        NOpenSSL::TEvpMdCtx ctx;
        UNIT_ASSERT(ctx);
    }
}
