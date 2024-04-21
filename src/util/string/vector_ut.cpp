#include <src/library/testing/unittest/registar.h>

#include "cast.h"
#include "vector.h"

Y_UNIT_TEST_SUITE(TStringJoinTest) {
    Y_UNIT_TEST(Test1) {
        std::vector<std::u16string> v;

        UNIT_ASSERT_EQUAL(JoinStrings(v, ToWtring("")), ToWtring(""));
    }

    Y_UNIT_TEST(Test2) {
        std::vector<std::u16string> v;

        v.push_back(ToWtring("1"));
        v.push_back(ToWtring("2"));

        UNIT_ASSERT_EQUAL(JoinStrings(v, ToWtring(" ")), ToWtring("1 2"));
    }

    Y_UNIT_TEST(Test3) {
        std::vector<std::u16string> v;

        v.push_back(ToWtring("1"));
        v.push_back(ToWtring("2"));

        UNIT_ASSERT_EQUAL(JoinStrings(v, 1, 10, ToWtring(" ")), ToWtring("2"));
    }

    Y_UNIT_TEST(TestJoinWStrings) {
        const std::u16string str = u"Яндекс";
        const std::vector<std::u16string> v(1, str);

        UNIT_ASSERT_EQUAL(JoinStrings(v, std::u16string()), str);
    }
}
