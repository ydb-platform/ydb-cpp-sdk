#include "execpath.h"

#include <library/cpp/testing/unittest/registar.h>

#include <util/folder/dirut.h>

Y_UNIT_TEST_SUITE(TExecPathTest) {
    Y_UNIT_TEST(TestIt) {
        std::string execPath = GetExecPath();
        std::string persistentExecPath = GetPersistentExecPath();

        try {
            UNIT_ASSERT(NFs::Exists(execPath));
            UNIT_ASSERT(NFs::Exists(persistentExecPath));
        } catch (...) {
            Cerr << execPath << Endl;

            throw;
        }
    }
}
