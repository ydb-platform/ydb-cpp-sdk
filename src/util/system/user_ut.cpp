#include "user.h"

#include <src/library/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TestUser) {
    Y_UNIT_TEST(TestNotEmpty) {
        UNIT_ASSERT(GetUsername());
    }
}
