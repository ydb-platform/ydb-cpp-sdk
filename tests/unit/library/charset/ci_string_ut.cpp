#include "ci_string.h"

#include <library/cpp/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TCiStringTest) {
    Y_UNIT_TEST(TestOperators) {
        TCiString s("0123456");

        const auto x = "x";
        const auto y = "y";
        const auto z = "z";

        // operator +=
        s += TCiString(x);
        UNIT_ASSERT(s == "0123456x");

        s += y;
        UNIT_ASSERT(s == "0123456xy");

        s += *z;
        UNIT_ASSERT(s == "0123456xyz");

        // operator +
        s = "0123456";
        s = s + TCiString(x);
        UNIT_ASSERT(s == "0123456x");

        s = s + y;
        UNIT_ASSERT(s == "0123456xy");

        s = s + *z;
        UNIT_ASSERT(s == "0123456xyz");

        // operator !=
        s = "012345";
        const auto xyz = "xyz";
        UNIT_ASSERT(s != TCiString(xyz));
        UNIT_ASSERT(s != xyz);
        UNIT_ASSERT(xyz != s);

        // operator <
        UNIT_ASSERT_EQUAL(s < TCiString(xyz), true);
        UNIT_ASSERT_EQUAL(s < xyz, true);
        UNIT_ASSERT_EQUAL(xyz < s, false);

        // operator <=
        UNIT_ASSERT_EQUAL(s <= TCiString(xyz), true);
        UNIT_ASSERT_EQUAL(s <= xyz, true);
        UNIT_ASSERT_EQUAL(xyz <= s, false);

        // operator >
        UNIT_ASSERT_EQUAL(s > TCiString(xyz), false);
        UNIT_ASSERT_EQUAL(s > xyz, false);
        UNIT_ASSERT_EQUAL(xyz > s, true);

        // operator >=
        UNIT_ASSERT_EQUAL(s >= TCiString(xyz), false);
        UNIT_ASSERT_EQUAL(s >= xyz, false);
        UNIT_ASSERT_EQUAL(xyz >= s, true);
    }

    Y_UNIT_TEST(TestOperatorsCI) {
        TCiString s("ABCD");
        UNIT_ASSERT(s > "abc0123456xyz");
        UNIT_ASSERT("abc0123456xyz" < s);
        UNIT_ASSERT(s == "abcd");
        UNIT_ASSERT("abcd" == s);
    }

    Y_UNIT_TEST(TestSpecial) {
        TCiString ss = "0123456"; // type 'TCiString' is used as is
        size_t hash_val = THash<TCiString>{}(ss);
        UNIT_ASSERT(hash_val == 1489244);
    }
}
