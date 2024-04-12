#include "strip.h"

#include <src/library/testing/unittest/registar.h>

#include <util/charset/wide.h>

Y_UNIT_TEST_SUITE(TStripStringTest) {
    struct TStripTest {
        std::string_view Str;
        std::string_view StripLeftRes;
        std::string_view StripRightRes;
        std::string_view StripRes;
    };
    static constexpr TStripTest StripTests[] = {
        {"  012  ", "012  ", "  012", "012"},
        {"  012", "012", "  012", "012"},
        {"012\t\t", "012\t\t", "012", "012"},
        {"\t012\t", "012\t", "\t012", "012"},
        {"012", "012", "012", "012"},
        {"012\r\n", "012\r\n", "012", "012"},
        {"\n012\r", "012\r", "\n012", "012"},
        {"\n \t\r", "", "", ""},
        {"", "", "", ""},
        {"abc", "abc", "abc", "abc"},
        {"a c", "a c", "a c", "a c"},
        {"  long string to avoid SSO            \n", "long string to avoid SSO            \n", "  long string to avoid SSO", "long string to avoid SSO"},
    };

    Y_UNIT_TEST(TestStrip) {
        for (const auto& test : StripTests) {
            std::string inputStr(test.Str);

            std::string s;
            Strip(inputStr, s);
            UNIT_ASSERT_EQUAL(s, test.StripRes);

            UNIT_ASSERT_EQUAL(StripString(inputStr), test.StripRes);
            UNIT_ASSERT_EQUAL(StripStringLeft(inputStr), test.StripLeftRes);
            UNIT_ASSERT_EQUAL(StripStringRight(inputStr), test.StripRightRes);

            std::string_view inputStrBuf(test.Str);
            UNIT_ASSERT_EQUAL(StripString(inputStrBuf), test.StripRes);
            UNIT_ASSERT_EQUAL(StripStringLeft(inputStrBuf), test.StripLeftRes);
            UNIT_ASSERT_EQUAL(StripStringRight(inputStrBuf), test.StripRightRes);
        };
    }

    Y_UNIT_TEST(TestStripInPlace) {
        for (const auto& test : StripTests) {
            std::string str(test.Str);
            Y_ASSERT(str.IsDetached() || str.empty()); // prerequisite of the test; check that we don't try to modify shared COW-string in-place by accident
            const void* stringPtrPrior = str.data();
            StripInPlace(str);
            const void* stringPtrAfter = str.data();
            UNIT_ASSERT_VALUES_EQUAL(str, test.StripRes);
            if (!test.Str.empty()) {
                UNIT_ASSERT_EQUAL_C(stringPtrPrior, stringPtrAfter, std::string(test.Str).Quote()); // StripInPlace should reuse buffer of original string
            }
        }
    }

    Y_UNIT_TEST(TestCustomStrip) {
        struct TTest {
            const char* Str;
            const char* Result;
        };
        static const TTest tests[] = {
            {"//012//", "012"},
            {"//012", "012"},
            {"012", "012"},
            {"012//", "012"},
        };

        for (auto test : tests) {
            UNIT_ASSERT_EQUAL(
                StripString(std::string(test.Str), EqualsStripAdapter('/')),
                test.Result);
        };
    }

    Y_UNIT_TEST(TestCustomStripLeftRight) {
        struct TTest {
            const char* Str;
            const char* ResultLeft;
            const char* ResultRight;
        };
        static const TTest tests[] = {
            {"//012//", "012//", "//012"},
            {"//012", "012", "//012"},
            {"012", "012", "012"},
            {"012//", "012//", "012"},
        };

        for (const auto& test : tests) {
            UNIT_ASSERT_EQUAL(
                StripStringLeft(std::string(test.Str), EqualsStripAdapter('/')),
                test.ResultLeft);
            UNIT_ASSERT_EQUAL(
                StripStringRight(std::string(test.Str), EqualsStripAdapter('/')),
                test.ResultRight);
        };
    }

    Y_UNIT_TEST(TestNullStringStrip) {
        std::string_view nullString(nullptr, nullptr);
        UNIT_ASSERT_EQUAL(
            StripString(nullString),
            std::string());
    }

    Y_UNIT_TEST(TestWtrokaStrip) {
        UNIT_ASSERT_EQUAL(StripString(std::u16string_view(u" abc ")), u"abc");
        UNIT_ASSERT_EQUAL(StripStringLeft(std::u16string_view(u" abc ")), u"abc ");
        UNIT_ASSERT_EQUAL(StripStringRight(std::u16string_view(u" abc ")), u" abc");
    }

    Y_UNIT_TEST(TestWtrokaCustomStrip) {
        UNIT_ASSERT_EQUAL(
            StripString(
                std::u16string_view(u"/abc/"),
                EqualsStripAdapter(u'/')),
            u"abc");
    }

    Y_UNIT_TEST(TestCollapseUtf32) {
        std::u32string s;
        Collapse(UTF8ToUTF32<true>("  123    456  "), s, IsWhitespace);
        UNIT_ASSERT(s == UTF8ToUTF32<true>(" 123 456 "));
        Collapse(UTF8ToUTF32<true>("  123    456  "), s, IsWhitespace, 10);
        UNIT_ASSERT(s == UTF8ToUTF32<true>(" 123 456  "));

        s = UTF8ToUTF32<true>(" a b c ");
        std::u32string s2 = s;
        CollapseInPlace(s2, IsWhitespace);

        UNIT_ASSERT(s == s2);
    }

    Y_UNIT_TEST(TestCollapseUtf16) {
        std::u16string s;
        Collapse(UTF8ToWide<true>("  123    456  "), s);
        UNIT_ASSERT(s == UTF8ToWide<true>(" 123 456 "));
        Collapse(UTF8ToWide<true>("  123    456  "), s, 10);
        UNIT_ASSERT(s == UTF8ToWide<true>(" 123 456  "));

        s = UTF8ToWide<true>(" a b c ");
        std::u16string s2 = s;
        CollapseInPlace(s2);

        UNIT_ASSERT(s == s2);
    }

    Y_UNIT_TEST(TestCollapse) {
        std::string s;
        Collapse(std::string("  123    456  "), s);
        UNIT_ASSERT(s == " 123 456 ");
        Collapse(std::string("  123    456  "), s, 10);
        UNIT_ASSERT(s == " 123 456  ");

        s = std::string(" a b c ");
        std::string s2 = s;
        CollapseInPlace(s2);

        UNIT_ASSERT(s == s2);
    }

    Y_UNIT_TEST(TestCollapseText) {
        std::string abs1("Very long description string written in unknown language.");
        std::string abs2(abs1);
        std::string abs3(abs1);
        CollapseText(abs1, 204);
        CollapseText(abs2, 54);
        CollapseText(abs3, 49);
        UNIT_ASSERT_EQUAL(abs1 == "Very long description string written in unknown language.", true);
        UNIT_ASSERT_EQUAL(abs2 == "Very long description string written in unknown ...", true);
        UNIT_ASSERT_EQUAL(abs3 == "Very long description string written in ...", true);
    }
}
