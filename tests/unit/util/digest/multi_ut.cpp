#include <ydb-cpp-sdk/util/digest/multi.h>

#include <library/cpp/testing/unittest/registar.h>

using namespace std::string_literals;

class TMultiHashTest: public TTestBase {
    UNIT_TEST_SUITE(TMultiHashTest);
    UNIT_TEST(TestStrInt)
    UNIT_TEST(TestIntStr)
    UNIT_TEST(TestSimpleCollision)
    UNIT_TEST(TestTypes)
    UNIT_TEST_SUITE_END();

private:
    inline void TestStrInt() {
        UNIT_ASSERT_EQUAL(MultiHash("1234567"s, 123), static_cast<size_t>(ULL(17038203285960021630)));
    }

    inline void TestIntStr() {
        UNIT_ASSERT_EQUAL(MultiHash(123, "1234567"s), static_cast<size_t>(ULL(9973288649881090712)));
    }

    inline void TestSimpleCollision() {
        UNIT_ASSERT_UNEQUAL(MultiHash(1, 1, 0), MultiHash(2, 2, 0));
    }

    inline void TestTypes() {
        UNIT_ASSERT_EQUAL(MultiHash("aaa", static_cast<ui64>(123)), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", static_cast<i64>(123)), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", static_cast<i32>(123)), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", static_cast<ui32>(123)), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", static_cast<i64>(-123)), MultiHash("aaa", -123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", static_cast<i32>(-123)), MultiHash("aaa", -123));
    }
};

UNIT_TEST_SUITE_REGISTRATION(TMultiHashTest);
