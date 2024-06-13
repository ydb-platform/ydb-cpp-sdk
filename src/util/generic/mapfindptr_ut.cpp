#include "mapfindptr.h"

#include <src/library/testing/unittest/registar.h>

#include <map>
#include <unordered_map>

Y_UNIT_TEST_SUITE(TMapFindPtrTest) {
    using namespace std::string_view_literals;

    using TTestMap = std::map<int, std::string>;

    Y_UNIT_TEST(TestDerivedClass) {
        TTestMap a;

        a[42] = "cat";
        UNIT_ASSERT(MapFindPtr(a, 42));
        UNIT_ASSERT_EQUAL(*MapFindPtr(a, 42), "cat");
        UNIT_ASSERT_EQUAL(MapFindPtr(a, 0), nullptr);

        //test mutation
        if (std::string* p = MapFindPtr(a, 42)) {
            *p = "dog";
        }
        UNIT_ASSERT(MapFindPtr(a, 42));
        UNIT_ASSERT_EQUAL(*MapFindPtr(a, 42), "dog");

        //test const-overloaded functions too
        const TTestMap& b = a;
        UNIT_ASSERT(MapFindPtr(b, 42) && *MapFindPtr(b, 42) == "dog");
        UNIT_ASSERT_EQUAL(MapFindPtr(b, 0), nullptr);

        UNIT_ASSERT_STRINGS_EQUAL(MapValue(b, 42, "cat"), "dog");
        UNIT_ASSERT_STRINGS_EQUAL(MapValue(b, 0, "alien"), "alien");
    }

    Y_UNIT_TEST(TestTemplateFind) {
        std::unordered_map<std::string, int, THash<std::string>, TEqualTo<std::string>> m;

        m[std::string("x")] = 2;

        UNIT_ASSERT(MapFindPtr(m, std::string_view("x")));
        UNIT_ASSERT_EQUAL(*MapFindPtr(m, std::string_view("x")), 2);
    }

    Y_UNIT_TEST(TestValue) {
        TTestMap a;

        a[1] = "lol";

        UNIT_ASSERT_VALUES_EQUAL(MapValue(a, 1, "123"), "lol");
        UNIT_ASSERT_VALUES_EQUAL(MapValue(a, 2, "123"), "123");
        UNIT_ASSERT_VALUES_EQUAL(MapValue(a, 2, "123"sv), "123"sv);
    }
}
