#include "list.h"

#include <library/cpp/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TYListSuite) {
    Y_UNIT_TEST(TestInitializerList) {
        std::list<int> l = {3, 42, 6};
        std::list<int> expected;
        expected.push_back(3);
        expected.push_back(42);
        expected.push_back(6);
        UNIT_ASSERT_VALUES_EQUAL(l, expected);
    }
}
