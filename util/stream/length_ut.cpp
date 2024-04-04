#include "length.h"

#include <library/cpp/testing/unittest/registar.h>


Y_UNIT_TEST_SUITE(TestLengthIO) {
    Y_UNIT_TEST(TestLengthLimitedInput) {
        char buf[16];

        TStringStream s1("abcd");
        TLengthLimitedInput l1(&s1, 2);
        UNIT_ASSERT_VALUES_EQUAL(l1.Load(buf, 3), 2);
        UNIT_ASSERT_VALUES_EQUAL(l1.Read(buf, 1), 0);
    }
}
