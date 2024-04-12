#include "compat.h"

#include <src/library/testing/unittest/registar.h>

#include <src/util/folder/dirut.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

Y_UNIT_TEST_SUITE(TCompatTest) {
    Y_UNIT_TEST(TestGetprogname) {
        getprogname(); // just check it links
    }
}
