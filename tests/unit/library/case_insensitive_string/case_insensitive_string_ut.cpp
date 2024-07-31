#include "case_insensitive_string.h"

#include <library/cpp/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TCaseInsensitiveCharTraits) {
    Y_UNIT_TEST(EqualTo) {
        constexpr auto eq = TCaseInsensitiveCharTraits::eq;
        constexpr auto eq_int_type = TCaseInsensitiveCharTraits::eq_int_type;
        constexpr auto to_int_type = TCaseInsensitiveCharTraits::to_int_type;
        constexpr auto eof = TCaseInsensitiveCharTraits::eof;

        const std::tuple<char, char, bool> cases[] = {
            {'\0', '\0', true},
            {'\0', 'a', false},
            {'a', 'a', true},
            {'a', 'b', false},
        };

        for (const auto& [c1, c2, answer] : cases) {
            const auto uc1 = static_cast<char>(std::toupper(static_cast<unsigned char>(c1)));
            const auto uc2 = static_cast<char>(std::toupper(static_cast<unsigned char>(c2)));

            UNIT_ASSERT_EQUAL(eq(c1, c2), answer);
            UNIT_ASSERT_EQUAL(eq(c2, c1), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(c1), to_int_type(c2)), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(c2), to_int_type(c1)), answer);

            UNIT_ASSERT_EQUAL(eq(c1, uc2), answer);
            UNIT_ASSERT_EQUAL(eq(uc2, c1), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(c1), to_int_type(uc2)), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(uc2), to_int_type(c1)), answer);

            UNIT_ASSERT_EQUAL(eq(uc1, c2), answer);
            UNIT_ASSERT_EQUAL(eq(c2, uc1), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(uc1), to_int_type(c2)), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(c2), to_int_type(uc1)), answer);

            UNIT_ASSERT_EQUAL(eq(uc1, uc2), answer);
            UNIT_ASSERT_EQUAL(eq(uc2, uc1), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(uc1), to_int_type(uc2)), answer);
            UNIT_ASSERT_EQUAL(eq_int_type(to_int_type(uc2), to_int_type(uc1)), answer);

            UNIT_ASSERT(!eq_int_type(eof(), to_int_type(c1)));
            UNIT_ASSERT(!eq_int_type(eof(), to_int_type(uc1)));
            UNIT_ASSERT(!eq_int_type(to_int_type(c1), eof()));
            UNIT_ASSERT(!eq_int_type(to_int_type(uc1), eof()));
            UNIT_ASSERT(!eq_int_type(eof(), to_int_type(c2)));
            UNIT_ASSERT(!eq_int_type(eof(), to_int_type(uc2)));
            UNIT_ASSERT(!eq_int_type(to_int_type(c2), eof()));
            UNIT_ASSERT(!eq_int_type(to_int_type(uc2), eof()));
            UNIT_ASSERT(eq_int_type(eof(), eof()));
        }
    }

    Y_UNIT_TEST(LessThan) {
        constexpr auto lt = TCaseInsensitiveCharTraits::lt;

        const std::tuple<char, char, bool> cases[] = {
            {'\0', '\0', false},
            {'\0', 'a', true},
            {'a', '\0', false},
            {'a', 'a', false},
            {'a', 'b', true},
            {'b', 'a', false},
        };

        for (const auto& [c1, c2, answer] : cases) {
            const auto uc1 = static_cast<char>(std::toupper(static_cast<unsigned char>(c1)));
            const auto uc2 = static_cast<char>(std::toupper(static_cast<unsigned char>(c2)));

            UNIT_ASSERT_EQUAL(lt(c1, c2), answer);
            UNIT_ASSERT_EQUAL(lt(c1, uc2), answer);
            UNIT_ASSERT_EQUAL(lt(uc1, c2), answer);
            UNIT_ASSERT_EQUAL(lt(uc1, uc2), answer);
        }
    }

    Y_UNIT_TEST(Compare) {
        constexpr auto compare = TCaseInsensitiveCharTraits::compare;

        const std::tuple<const char*, const char*, std::size_t> equal_cases[] = {
            {"", "", 1},
            {"foo", "FOO", 4},
            {"Bar", "bAR", 4},
        };

        for (const auto& [s1, s2, size] : equal_cases) {
            for (std::size_t n = 0; n < size; ++n) {
                UNIT_ASSERT_EQUAL(compare(s1, s1, n), 0);
                UNIT_ASSERT_EQUAL(compare(s2, s2, n), 0);
                UNIT_ASSERT_EQUAL(compare(s1, s2, n), 0);
                UNIT_ASSERT_EQUAL(compare(s2, s1, n), 0);
            }
        }

        const std::tuple<const char*, std::size_t, const char*, std::size_t> non_equal_cases[] = {
            {"", 1, "foo", 4},
            {"foo", 4, "foobar", 7},
        };

        for (const auto& [s1, size_1, s2, size_2] : non_equal_cases) {
            for (std::size_t n = std::min(size_1, size_2); n < std::max(size_1, size_2); ++n) {
                UNIT_ASSERT_LT(compare(s1, s2, n), 0);
                UNIT_ASSERT_GT(compare(s2, s1, n), 0);
            }
        }
    }

    Y_UNIT_TEST(Find) {
        constexpr auto find = TCaseInsensitiveCharTraits::find;

        const auto s1 = "foobar";
        const auto s2 = "FOOBAR";
        constexpr std::size_t size = 7;

        UNIT_ASSERT_EQUAL(find(s1, size, 'x'), nullptr);
        UNIT_ASSERT_EQUAL(find(s1, size, 'X'), nullptr);

        const std::tuple<char, std::size_t> cases[] = {
            {'f', 0},
            {'o', 1},
            {'b', 3},
            {'a', 4},
            {'r', 5},
            {'\0', 6},
        };

        for (const auto& [ch, offset] : cases) {
            const auto uch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));

            UNIT_ASSERT_EQUAL(find(s1, 0, ch), nullptr);
            UNIT_ASSERT_EQUAL(find(s1, 0, uch), nullptr);

            UNIT_ASSERT_EQUAL(find(s2, 0, ch), nullptr);
            UNIT_ASSERT_EQUAL(find(s2, 0, uch), nullptr);

            UNIT_ASSERT_EQUAL(find(s1, offset, ch), nullptr);
            UNIT_ASSERT_EQUAL(find(s1, offset, uch), nullptr);

            UNIT_ASSERT_EQUAL(find(s2, offset, ch), nullptr);
            UNIT_ASSERT_EQUAL(find(s2, offset, uch), nullptr);

            UNIT_ASSERT_EQUAL(find(s1, size, ch), s1 + offset);
            UNIT_ASSERT_EQUAL(find(s1, size, uch), s1 + offset);

            UNIT_ASSERT_EQUAL(find(s2, size, ch), s2 + offset);
            UNIT_ASSERT_EQUAL(find(s2, size, uch), s2 + offset);
        }
    }
}

Y_UNIT_TEST_SUITE(TCaseInsensitiveStringTest) {
    using TCIString = TCaseInsensitiveString;
    using TCIStringBuf = TCaseInsensitiveStringBuf;

    Y_UNIT_TEST(TestOperators) {
        TCIString s("0123456");

        const auto x = "x";
        const auto y = "y";
        const auto z = "z";

        // operator +=
        s += TCIString(x);
        UNIT_ASSERT(s == "0123456x");

        s += y;
        UNIT_ASSERT(s == "0123456xy");

        s += *z;
        UNIT_ASSERT(s == "0123456xyz");

        // operator +
        s = "0123456";
        s = s + TCIString(x);
        UNIT_ASSERT(s == "0123456x");

        s = s + y;
        UNIT_ASSERT(s == "0123456xy");

        s = s + *z;
        UNIT_ASSERT(s == "0123456xyz");

        // operator !=
        s = "012345";
        const auto xyz = "xyz";
        UNIT_ASSERT(s != TCIString(xyz));
        UNIT_ASSERT(s != xyz);
        UNIT_ASSERT(xyz != s);

        // operator <
        UNIT_ASSERT_EQUAL(s < TCIString(xyz), true);
        UNIT_ASSERT_EQUAL(s < xyz, true);
        UNIT_ASSERT_EQUAL(xyz < s, false);

        // operator <=
        UNIT_ASSERT_EQUAL(s <= TCIString(xyz), true);
        UNIT_ASSERT_EQUAL(s <= xyz, true);
        UNIT_ASSERT_EQUAL(xyz <= s, false);

        // operator >
        UNIT_ASSERT_EQUAL(s > TCIString(xyz), false);
        UNIT_ASSERT_EQUAL(s > xyz, false);
        UNIT_ASSERT_EQUAL(xyz > s, true);

        // operator >=
        UNIT_ASSERT_EQUAL(s >= TCIString(xyz), false);
        UNIT_ASSERT_EQUAL(s >= xyz, false);
        UNIT_ASSERT_EQUAL(xyz >= s, true);
    }

    Y_UNIT_TEST(TestOperatorsCI) {
        TCIString s("ABCD");
        UNIT_ASSERT(s > "abc0123456xyz");
        UNIT_ASSERT(s == "abcd");
        UNIT_ASSERT(s > TCIStringBuf("abc0123456xyz"));
        UNIT_ASSERT(TCIStringBuf("abc0123456xyz") < s);
        UNIT_ASSERT(s == TCIStringBuf("abcd"));
    }

    Y_UNIT_TEST(BasicStdString) {
        TCIString foo("foo");
        TCIString FOO("FOO");
        TCIString Bar("Bar");
        TCIString bAR("bAR");

        UNIT_ASSERT_EQUAL(foo, FOO);
        UNIT_ASSERT_EQUAL(Bar, bAR);

        constexpr TCIStringBuf foobar("foobar");
        UNIT_ASSERT(foobar.starts_with(foo));
        UNIT_ASSERT(foobar.starts_with(FOO));
        UNIT_ASSERT(foobar.ends_with(Bar));
        UNIT_ASSERT(foobar.ends_with(bAR));
    #if __cpp_lib_string_contains >= 202011L
        UNIT_ASSERT(foobar.contains(FOO));
        UNIT_ASSERT(foobar.contains(Bar));
    #endif
    }

// StringSplitter does not use CharTraits properly
// Splitting such strings is explicitly disabled.
#if 0
    Y_UNIT_TEST(TestSplit) {
        TCIStringBuf input("splitAmeAbro");
        std::vector<TCIStringBuf> expected{"split", "me", "bro"};

        std::vector<TCIStringBuf> split = StringSplitter(input).Split('a');

        UNIT_ASSERT_VALUES_EQUAL(split, expected);
    }
#endif
}
