#include "string.h"
#include "hash.h"

#include <src/library/testing/unittest/registar.h>

#include <map>

#include "mapfindptr.h"

Y_UNIT_TEST_SUITE(TMapFindPtrTest) {
    struct TTestMap: std::map<int, std::string>, TMapOps<TTestMap> {};

    Y_UNIT_TEST(TestDerivedClass) {
        TTestMap a;

        a[42] = "cat";
        UNIT_ASSERT(a.FindPtr(42));
        UNIT_ASSERT_EQUAL(*a.FindPtr(42), "cat");
        UNIT_ASSERT_EQUAL(a.FindPtr(0), nullptr);

        //test mutation
        if (std::string* p = a.FindPtr(42)) {
            *p = "dog";
        }
        UNIT_ASSERT(a.FindPtr(42));
        UNIT_ASSERT_EQUAL(*a.FindPtr(42), "dog");

        //test const-overloaded functions too
        const TTestMap& b = a;
        UNIT_ASSERT(b.FindPtr(42) && *b.FindPtr(42) == "dog");
        UNIT_ASSERT_EQUAL(b.FindPtr(0), nullptr);

        UNIT_ASSERT_STRINGS_EQUAL(b.Value(42, "cat"), "dog");
        UNIT_ASSERT_STRINGS_EQUAL(b.Value(0, "alien"), "alien");
    }

    Y_UNIT_TEST(TestTemplateFind) {
        THashMap<std::string, int> m;

        m[std::string("x")] = 2;

        UNIT_ASSERT(m.FindPtr(std::string_view("x")));
        UNIT_ASSERT_EQUAL(*m.FindPtr(std::string_view("x")), 2);
    }

    Y_UNIT_TEST(TestValue) {
        TTestMap a;

        a[1] = "lol";

        UNIT_ASSERT_VALUES_EQUAL(a.Value(1, "123"), "lol");
        UNIT_ASSERT_VALUES_EQUAL(a.Value(2, "123"), "123");
        UNIT_ASSERT_VALUES_EQUAL(a.Value(2, "123"sv), "123"sv);
    }

    Y_UNIT_TEST(TestValueRef) {
        TTestMap a;

        a[1] = "lol";

        const std::string str123 = "123";
        std::string str1234 = "1234";

        UNIT_ASSERT_VALUES_EQUAL(a.ValueRef(1, str123), "lol");
        UNIT_ASSERT_VALUES_EQUAL(a.ValueRef(2, str123), "123");
        UNIT_ASSERT_VALUES_EQUAL(a.ValueRef(3, str1234), "1234");
    }
}
