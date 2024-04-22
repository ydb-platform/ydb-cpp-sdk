#include "compat.h"

#include <src/library/testing/unittest/registar.h>

#include <src/util/folder/dirut.h>
#include <ydb-cpp-sdk/util/stream/output.h>

Y_UNIT_TEST_SUITE(TCompatTest) {
    Y_UNIT_TEST(TestGetprogname) {
        getprogname(); // just check it links
    }
}
