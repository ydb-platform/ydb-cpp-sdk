#include "progname.h"

#include <library/cpp/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TProgramNameTest) {
    Y_UNIT_TEST(TestIt) {
        std::string progName = GetProgramName();

        try {
            UNIT_ASSERT(
                progName.find("ut_util") != std::string::npos || progName.find("util-system_ut") != std::string::npos || progName.find("util-system-ut") != std::string::npos);
        } catch (...) {
            Cerr << progName << Endl;

            throw;
        }
    }
}
