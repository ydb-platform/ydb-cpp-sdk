#include <src/library/testing/common/probe.h>
#include <src/library/testing/unittest/registar.h>

#include <ydb-cpp-sdk/util/str_stl.h>
#include <ydb-cpp-sdk/util/digest/multi.h>

#include <utility>

class THashTest: public TTestBase {
    UNIT_TEST_SUITE(THashTest);
    UNIT_TEST(TestTupleHash);
    UNIT_TEST(TestStringHash);
    UNIT_TEST_SUITE_END();

protected:
    void TestTupleHash();
    void TestStringHash();
};

UNIT_TEST_SUITE_REGISTRATION(THashTest);

/*
 * Sequence for MultiHash is reversed as it calculates hash as
 * f(head:tail) = f(tail)xHash(head)
 */
void THashTest::TestTupleHash() {
    std::tuple<int, int> tuple{1, 3};
    UNIT_ASSERT_VALUES_EQUAL(THash<decltype(tuple)>()(tuple), MultiHash(3, 1));

    /*
     * This thing checks that we didn't break STL code
     * See https://a.yandex-team.ru/arc/commit/2864838#comment-401
     * for example
     */
    struct A {
        A Foo(const std::tuple<A, float>& v) {
            return std::get<A>(v);
        }
    };
}

void THashTest::TestStringHash() {
    using namespace std::string_view_literals;

    // Make sure that different THash<> variants behave in the same way
    const size_t expected = THash<std::string>{}(std::string("hehe"));
    UNIT_ASSERT_VALUES_EQUAL(THash<const char[5]>{}("hehe"), expected);                       // char[5]
    UNIT_ASSERT_VALUES_EQUAL(THash<std::string_view>{}("hehe"sv), expected);                  // std::string_view
    UNIT_ASSERT_VALUES_EQUAL(THash<std::string_view>{}(std::string_view("hehe")), expected);  // std::string_view
    UNIT_ASSERT_VALUES_EQUAL(THash<const char*>{}("hehe"), expected);                         // const char*
}
