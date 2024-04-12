#include "thread_helper.h"

#include <src/library/testing/unittest/registar.h>

#include <string>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

Y_UNIT_TEST_SUITE(TestMP) {
    Y_UNIT_TEST(TestErr) {
        std::function<void(int)> f = [](int x) {
            if (x == 5) {
                ythrow yexception() << "oops";
            }
        };

        std::string s;

        try {
            NYmp::ParallelForStaticAutoChunk(0, 10, f);
        } catch (...) {
            s = CurrentExceptionMessage();
        }

        UNIT_ASSERT(s.find("oops") > 0);
    }
}
