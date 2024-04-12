#include "hmac.h"

#include <src/library/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(Hmac) {
    Y_UNIT_TEST(Ctx) {
        NOpenSSL::THmacCtx ctx;
        UNIT_ASSERT(ctx);
    }
}
