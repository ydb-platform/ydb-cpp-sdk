#include "multi.h"

#include <src/library/testing/unittest/registar.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TMultiHashTest: public TTestBase {
    UNIT_TEST_SUITE(TMultiHashTest);
    UNIT_TEST(TestStrInt)
    UNIT_TEST(TestIntStr)
    UNIT_TEST(TestSimpleCollision)
    UNIT_TEST(TestTypes)
    UNIT_TEST_SUITE_END();

private:
    inline void TestStrInt() {
        UNIT_ASSERT_EQUAL(MultiHash(std::string("1234567"), static_cast<int>(123)), static_cast<size_t>(ULL(17038203285960021630)));
    }

    inline void TestIntStr() {
        UNIT_ASSERT_EQUAL(MultiHash(static_cast<int>(123), std::string("1234567")), static_cast<size_t>(ULL(9973288649881090712)));
    }

    inline void TestSimpleCollision() {
        UNIT_ASSERT_UNEQUAL(MultiHash(1, 1, 0), MultiHash(2, 2, 0));
    }

    inline void TestTypes() {
        UNIT_ASSERT_EQUAL(MultiHash("aaa", (ui64)123), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", (i64)123), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", (i32)123), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", (ui32)123), MultiHash("aaa", 123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", (i64)-123), MultiHash("aaa", -123));
        UNIT_ASSERT_EQUAL(MultiHash("aaa", (i32)-123), MultiHash("aaa", -123));
    }
};

UNIT_TEST_SUITE_REGISTRATION(TMultiHashTest);
