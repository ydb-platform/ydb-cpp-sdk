#include "hostname.h"

#include <src/library/testing/unittest/registar.h>

#include <iostream>

Y_UNIT_TEST_SUITE(THostNameTest) {
    Y_UNIT_TEST(Test1) {
        UNIT_ASSERT(*GetHostName() != '?');
    }

    Y_UNIT_TEST(TestFQDN) {
        UNIT_ASSERT(*GetFQDNHostName() != '?');
    }

    Y_UNIT_TEST(TestIsFQDN) {
        const auto x = GetFQDNHostName();

        try {
            UNIT_ASSERT(IsFQDN(x));
        } catch (...) {
            std::cerr << x << std::endl;

            throw;
        }
    }
}
