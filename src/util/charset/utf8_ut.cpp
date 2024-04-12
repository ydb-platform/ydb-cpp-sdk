#include "utf8.h"
#include "wide.h"

#include <src/util/stream/file.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/ysaveload.h>
=======
#include <src/util/ysaveload.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <src/library/testing/unittest/registar.h>
#include <src/library/testing/unittest/env.h>

Y_UNIT_TEST_SUITE(TUtfUtilTest) {
    Y_UNIT_TEST(TestUTF8Len) {
        UNIT_ASSERT_EQUAL(GetNumberOfUTF8Chars("привет!"), 7);
    }

    Y_UNIT_TEST(TestToLowerUtfString) {
        UNIT_ASSERT_VALUES_EQUAL(ToLowerUTF8("xyz XYZ ПРИВЕТ!"), "xyz xyz привет!");

        UNIT_ASSERT_VALUES_EQUAL(ToLowerUTF8(std::string_view("xyz")), "xyz");

        {
            std::string s = "привет!";
            std::string q = "ПРИВЕТ!";
            std::string tmp;
            UNIT_ASSERT(ToLowerUTF8Impl(s.data(), s.size(), tmp) == false);
            UNIT_ASSERT(ToLowerUTF8Impl(q.data(), q.size(), tmp) == true);
        }

        {
            const char* weird = "\xC8\xBE"; // 'Ⱦ', U+023E. strlen(weird)==2, strlen(tolower_utf8(weird)) is 3
            const char* turkI = "İ";        //strlen("İ") == 2, strlen(tolower_utf8("İ") == 1
            std::string_view chars[] = {"f", "F", "Б", "б", weird, turkI};
            const int N = Y_ARRAY_SIZE(chars);
            //try all combinations of these letters.
            int numberOfVariants = 1;
            for (int len = 0; len <= 4; ++len) {
                for (int i = 0; i < numberOfVariants; ++i) {
                    std::string s;
                    int k = i;
                    for (int j = 0; j < len; ++j) {
                        //Treat 'i' like number in base-N system with digits from 'chars'-array
                        s += chars[k % N];
                        k /= N;
                    }

                    std::u16string tmp = UTF8ToWide(s);
                    tmp.to_lower();

                    UNIT_ASSERT_VALUES_EQUAL(ToLowerUTF8(s), WideToUTF8(tmp));
                }
                numberOfVariants *= N;
            }
        }
    }

    Y_UNIT_TEST(TestToUpperUtfString) {
        UNIT_ASSERT_VALUES_EQUAL(ToUpperUTF8("xyz XYZ привет!"), "XYZ XYZ ПРИВЕТ!");

        UNIT_ASSERT_VALUES_EQUAL(ToUpperUTF8(std::string_view("XYZ")), "XYZ");

        {
            std::string s = "ПРИВЕТ!";
            std::string q = "привет!";
            std::string tmp;
            UNIT_ASSERT(ToUpperUTF8Impl(s.data(), s.size(), tmp) == false);
            UNIT_ASSERT(ToUpperUTF8Impl(q.data(), q.size(), tmp) == true);
        }

        {
            const char* weird = "\xC8\xBE"; // 'Ⱦ', U+023E. strlen(weird)==2, strlen(ToUpper_utf8(weird)) is 3
            const char* turkI = "İ";        //strlen("İ") == 2, strlen(ToUpper_utf8("İ") == 1
            std::string_view chars[] = {"F", "f", "б", "Б", turkI, weird};
            const int N = Y_ARRAY_SIZE(chars);
            //try all combinations of these letters.
            int numberOfVariants = 1;
            for (int len = 0; len <= 4; ++len) {
                for (int i = 0; i < numberOfVariants; ++i) {
                    std::string s;
                    int k = i;
                    for (int j = 0; j < len; ++j) {
                        //Treat 'i' like number in base-N system with digits from 'chars'-array
                        s += chars[k % N];
                        k /= N;
                    }

                    std::u16string tmp = UTF8ToWide(s);
                    tmp.to_upper();

                    UNIT_ASSERT_VALUES_EQUAL(ToUpperUTF8(s), WideToUTF8(tmp));
                }
                numberOfVariants *= N;
            }
        }
    }

    Y_UNIT_TEST(TestUTF8ToWide) {
        TFileInput in(ArcadiaSourceRoot() + std::string_view("/src/util/charset/ut/utf8/test1.txt"));

        std::string text = in.ReadAll();
        UNIT_ASSERT(WideToUTF8(UTF8ToWide(text)) == text);
    }

    Y_UNIT_TEST(TestInvalidUTF8) {
        std::vector<std::string> testData;
        TFileInput input(ArcadiaSourceRoot() + std::string_view("/src/util/charset/ut/utf8/invalid_UTF8.bin"));
        Load(&input, testData);

        for (const auto& text : testData) {
            UNIT_ASSERT_EXCEPTION(UTF8ToWide(text), yexception);
        }
    }

    Y_UNIT_TEST(TestUTF8ToWideScalar) {
        TFileInput in(ArcadiaSourceRoot() + std::string_view("/src/util/charset/ut/utf8/test1.txt"));

        std::string text = in.ReadAll();
        std::u16string wtextSSE = UTF8ToWide(text);
        std::u16string wtextScalar = std::u16string::Uninitialized(text.size());
        const unsigned char* textBegin = reinterpret_cast<const unsigned char*>(text.c_str());
        wchar16* wtextBegin = wtextScalar.begin();
        ::NDetail::UTF8ToWideImplScalar<false>(textBegin, textBegin + text.size(), wtextBegin);
        UNIT_ASSERT(wtextBegin == wtextScalar.begin() + wtextSSE.size());
        UNIT_ASSERT(textBegin == reinterpret_cast<const unsigned char*>(text.end()));
        wtextScalar.remove(wtextSSE.size());
        UNIT_ASSERT(wtextScalar == wtextSSE);
    }
}
