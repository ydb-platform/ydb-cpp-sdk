#include "scope.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
#include <src/library/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(ScopeToolsTest) {
    Y_UNIT_TEST(OnScopeExitTest) {
        int i = 0;

        {
            Y_SCOPE_EXIT(&i) {
                i = i * 2;
            };

            Y_SCOPE_EXIT(&i) {
                i = i + 1;
            };
        }

        UNIT_ASSERT_VALUES_EQUAL(2, i);
    }

    Y_UNIT_TEST(OnScopeExitMoveTest) {
        THolder<int> i{new int{10}};
        int p = 0;

        {
            Y_SCOPE_EXIT(i = std::move(i), &p) {
                p = *i * 2;
            };
        }

        UNIT_ASSERT_VALUES_EQUAL(20, p);
    }

    Y_UNIT_TEST(TestDefer) {
        int i = 0;

        {
            Y_DEFER {
                i = 20;
            };
        }

        UNIT_ASSERT_VALUES_EQUAL(i, 20);
    }
}
