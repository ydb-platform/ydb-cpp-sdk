#include "case_insensitive_string.h"

#include <src/util/generic/string_ut.h>

class TCaseInsensitiveStringTest : public TTestBase, private TStringTestImpl<TCaseInsensitiveString, TTestData<char>> {
public:
    UNIT_TEST_SUITE(TCaseInsensitiveStringTest);
    UNIT_TEST(TestOperators);
    UNIT_TEST(TestOperatorsCI);

    UNIT_TEST_SUITE_END();
};

UNIT_TEST_SUITE_REGISTRATION(TCaseInsensitiveStringTest);

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

Y_UNIT_TEST_SUITE(TCaseInsensitiveStringTestEx) {
    Y_UNIT_TEST(BasicTString) {
        TCaseInsensitiveString foo("foo");
        TCaseInsensitiveString FOO("FOO");
        TCaseInsensitiveString Bar("Bar");
        TCaseInsensitiveString bAR("bAR");

        UNIT_ASSERT_EQUAL(foo, FOO);
        UNIT_ASSERT_EQUAL(Bar, bAR);

        constexpr TCaseInsensitiveStringBuf foobar("foobar");
        UNIT_ASSERT(foobar.StartsWith(foo));
        UNIT_ASSERT(foobar.StartsWith(FOO));
        UNIT_ASSERT(foobar.EndsWith(Bar));
        UNIT_ASSERT(foobar.EndsWith(bAR));
        UNIT_ASSERT(foobar.Contains(FOO));
        UNIT_ASSERT(foobar.Contains(Bar));
    }

    Y_UNIT_TEST(BasicStdString) {
        using TCaseInsensitiveStdString = std::basic_string<char, TCaseInsensitiveCharTraits>;
        using TCaseInsensitiveStringView = std::basic_string_view<char, TCaseInsensitiveCharTraits>;

        TCaseInsensitiveStdString foo("foo");
        TCaseInsensitiveStdString FOO("FOO");
        TCaseInsensitiveStdString Bar("Bar");
        TCaseInsensitiveStdString bAR("bAR");

        UNIT_ASSERT_EQUAL(foo, FOO);
        UNIT_ASSERT_EQUAL(Bar, bAR);

        constexpr TCaseInsensitiveStringView foobar("foobar");
        UNIT_ASSERT(foobar.starts_with(foo));
        UNIT_ASSERT(foobar.starts_with(FOO));
        UNIT_ASSERT(foobar.ends_with(Bar));
        UNIT_ASSERT(foobar.ends_with(bAR));
        //TODO: test contains after C++23
    }

/*
    Y_UNIT_TEST(TestSplit) {
        TCaseInsensitiveStringBuf input("splitAmeAbro");
        std::vector<TCaseInsensitiveStringBuf> expected{"split", "me", "bro"};

        std::vector<TCaseInsensitiveStringBuf> split = StringSplitter(input).Split('a');

        UNIT_ASSERT_VALUES_EQUAL(split, expected);
    }
*/
}
