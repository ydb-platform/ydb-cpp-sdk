#include "escape.h"

#include <library/cpp/testing/unittest/registar.h>

#include <util/charset/wide.h>

using namespace std::string_view_literals;

namespace {
    struct TExample {
        std::string Expected;
        std::string Source;

        TExample(const std::string_view expected, const std::string_view source)
            : Expected{expected}
            , Source{source}
        {
        }
    };
}

static const TExample CommonTestData[] = {
    // Should be valid UTF-8.
    {"http://ya.ru/", "http://ya.ru/"},
    {"http://ya.ru/\\x17\\n", "http://ya.ru/\x17\n"},

    {"http://ya.ru/\\0", "http://ya.ru/\0"sv},
    {"http://ya.ru/\\0\\0", "http://ya.ru/\0\0"sv},
    {"http://ya.ru/\\0\\0000", "http://ya.ru/\0\0"
                               "0"sv},
    {"http://ya.ru/\\0\\0001", "http://ya.ru/\0\x00"
                               "1"sv},

    {R"(\2\4\00678)", "\2\4\6"
                      "78"sv}, // \6 -> \006 because next char '7' is "octal"
    {R"(\2\4\689)", "\2\4\6"
                    "89"sv}, // \6 -> \6 because next char '8' is not "octal"

    {R"(\"Hello\", Alice said.)", "\"Hello\", Alice said."},
    {"Slash\\\\dash!", "Slash\\dash!"},
    {R"(There\nare\r\nnewlines.)", "There\nare\r\nnewlines."},
    {"There\\tare\\ttabs.", "There\tare\ttabs."},

    {"There are questions \\x3F\\x3F?", "There are questions ???"},
    {"There are questions \\x3F?", "There are questions ??"},
};

Y_UNIT_TEST_SUITE(TEscapeCTest) {
    Y_UNIT_TEST(TestStrokaEscapeC) {
        for (const auto& e : CommonTestData) {
            std::string expected(e.Expected);
            std::string source(e.Source);
            std::string actual(EscapeC(e.Source));
            std::string actual2(UnescapeC(e.Expected));

            UNIT_ASSERT_VALUES_EQUAL(e.Expected, actual);
            UNIT_ASSERT_VALUES_EQUAL(e.Source, actual2);
        }

        UNIT_ASSERT_VALUES_EQUAL("http://ya.ru/\\x17\\n\\xAB", EscapeC(std::string("http://ya.ru/\x17\n\xab")));
        UNIT_ASSERT_VALUES_EQUAL("http://ya.ru/\x17\n\xab", UnescapeC(std::string("http://ya.ru/\\x17\\n\\xAB")));
        UNIT_ASSERT_VALUES_EQUAL("h", EscapeC('h'));
        UNIT_ASSERT_VALUES_EQUAL("h", UnescapeC(std::string("h")));
        UNIT_ASSERT_VALUES_EQUAL("\\xFF", EscapeC('\xFF'));
        UNIT_ASSERT_VALUES_EQUAL("\xFF", UnescapeC(std::string("\\xFF")));

        UNIT_ASSERT_VALUES_EQUAL("\\377f", EscapeC(std::string("\xff"
                                                           "f")));
        UNIT_ASSERT_VALUES_EQUAL("\xff"
                                 "f",
                                 UnescapeC(std::string("\\377f")));
        UNIT_ASSERT_VALUES_EQUAL("\\xFFg", EscapeC(std::string("\xff"
                                                           "g")));
        UNIT_ASSERT_VALUES_EQUAL("\xff"
                                 "g",
                                 UnescapeC(std::string("\\xFFg")));
        UNIT_ASSERT_VALUES_EQUAL("\xEA\x9A\x96", UnescapeC(std::string("\\uA696")));
        UNIT_ASSERT_VALUES_EQUAL("Странный компроматтест", UnescapeC(std::string("\\u0421\\u0442\\u0440\\u0430\\u043d\\u043d\\u044b\\u0439 \\u043a\\u043e\\u043c\\u043f\\u0440\\u043e\\u043c\\u0430\\u0442тест")));
    }

    Y_UNIT_TEST(TestWtrokaEscapeC) {
        for (const auto& e : CommonTestData) {
            std::u16string expected(UTF8ToWide(e.Expected));
            std::u16string source(UTF8ToWide(e.Source));
            std::u16string actual(EscapeC(source));
            std::u16string actual2(UnescapeC(expected));

            UNIT_ASSERT_VALUES_EQUAL(expected, actual);
            UNIT_ASSERT_VALUES_EQUAL(source, actual2);
        }

        UNIT_ASSERT_VALUES_EQUAL(u"http://ya.ru/\\x17\\n\\u1234", EscapeC(u"http://ya.ru/\x17\n\u1234"));
        UNIT_ASSERT_VALUES_EQUAL(u"h", EscapeC(u'h'));
        UNIT_ASSERT_VALUES_EQUAL(u"\\xFF", EscapeC(wchar16(255)));
    }

    Y_UNIT_TEST(TestEscapeTrigraphs) {
        UNIT_ASSERT_VALUES_EQUAL("?", EscapeC(std::string("?")));
        UNIT_ASSERT_VALUES_EQUAL("\\x3F?", EscapeC(std::string("??")));
        UNIT_ASSERT_VALUES_EQUAL("\\x3F\\x3F?", EscapeC(std::string("???")));
        // ok but may cause warning about trigraphs
        // UNIT_ASSERT_VALUES_EQUAL("[x]?z", EscapeC(std::string("??(x??)?z")));
        UNIT_ASSERT_VALUES_EQUAL("\\x3F?x\\x3F\\x3F?z", EscapeC(std::string("??x???z")));
    }

    Y_UNIT_TEST(TestUnescapeCCharLen) {
        auto test = [](const char* str, size_t len) {
            UNIT_ASSERT_EQUAL(UnescapeCCharLen(str, str + strlen(str)), len);
        };

        test("", 0);
        test("abc", 1);
        test("\\", 1);
        test("\\\\", 2);
        test("\\#", 2);
        test("\\n10", 2);
        test("\\r\\n", 2);
        test("\\x05abc", 4);
        test("\\u11117777", 6);
        test("\\u123yyy", 2);
        test("\\U11117777cccc", 10);
        test("\\U111yyy", 2);
        test("\\0\\1", 2);
        test("\\01\\1", 3);
        test("\\012\\1", 4);
        test("\\0123\\1", 4);
        test("\\4\\1", 2);
        test("\\40\\1", 3);
        test("\\400\\1", 3);
        test("\\4xxx", 2);
    }

    Y_UNIT_TEST(TestUnbounded) {
        char buf[100000];

        for (const auto& x : CommonTestData) {
            char* end = UnescapeC(x.Expected.data(), x.Expected.size(), buf);

            UNIT_ASSERT_VALUES_EQUAL(x.Source, std::string_view(buf, end));
        }
    }

    Y_UNIT_TEST(TestCapitalUEscapes) {
        UNIT_ASSERT_VALUES_EQUAL(UnescapeC("\\U00000020"), " ");
        UNIT_ASSERT_VALUES_EQUAL(UnescapeC("\\Uxxx"), "Uxxx");
    }
}
