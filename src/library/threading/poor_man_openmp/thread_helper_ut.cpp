#include "thread_helper.h"

#include <src/library/testing/unittest/registar.h>

#include <string>
#include <util/generic/yexception.h>

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
