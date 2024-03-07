#include "printf.h"

#include <library/cpp/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(StringPrintf) {
    Y_UNIT_TEST(TestSprintf) {
        std::string s;
        int len = sprintf(s, "Hello %s", "world");
        UNIT_ASSERT_EQUAL(s, std::string("Hello world"));
        UNIT_ASSERT_EQUAL(len, 11);
    }

    Y_UNIT_TEST(TestFcat) {
        std::string s;
        int len = sprintf(s, "Hello %s", "world");
        UNIT_ASSERT_EQUAL(s, std::string("Hello world"));
        UNIT_ASSERT_EQUAL(len, 11);
        len = fcat(s, " qwqw%s", "as");
        UNIT_ASSERT_EQUAL(s, std::string("Hello world qwqwas"));
        UNIT_ASSERT_EQUAL(len, 7);
    }

    Y_UNIT_TEST(TestSpecial) {
        UNIT_ASSERT_EQUAL("4294967295", Sprintf("%" PRIu32, (ui32)(-1)));
    }

    Y_UNIT_TEST(TestExplicitPositions) {
        UNIT_ASSERT_EQUAL("abc xyz abc", Sprintf("%1$s %2$s %1$s", "abc", "xyz"));
    }
}
