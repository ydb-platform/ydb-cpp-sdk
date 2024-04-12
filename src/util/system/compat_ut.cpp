#include "compat.h"

#include <src/library/testing/unittest/registar.h>

#include <src/util/folder/dirut.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

Y_UNIT_TEST_SUITE(TCompatTest) {
    Y_UNIT_TEST(TestGetprogname) {
        getprogname(); // just check it links
    }
}
