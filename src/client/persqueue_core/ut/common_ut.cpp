#include <src/client/ydb_persqueue_core/impl/common.h>

#include <src/library/testing/unittest/registar.h>

using NYdb::NPersQueue::ApplyClusterEndpoint;

Y_UNIT_TEST_SUITE(ApplyClusterEndpointTest) {
    Y_UNIT_TEST(NoPorts) {
        UNIT_ASSERT_STRINGS_EQUAL(ApplyClusterEndpoint("driver", "cluster"), "cluster");
        UNIT_ASSERT_STRINGS_EQUAL(ApplyClusterEndpoint("driver", "clus:ter"), "clus:ter");
        UNIT_ASSERT_STRINGS_EQUAL(ApplyClusterEndpoint("dri:ver", "cluster"), "cluster");
    }

    Y_UNIT_TEST(PortFromCds) {
        UNIT_ASSERT_STRINGS_EQUAL(ApplyClusterEndpoint("driver", "cluster:80"), "cluster:80");
        UNIT_ASSERT_STRINGS_EQUAL(ApplyClusterEndpoint("driver:75", "cluster:80"), "cluster:80");
    }

    Y_UNIT_TEST(PortFromDriver) {
        UNIT_ASSERT_STRINGS_EQUAL(ApplyClusterEndpoint("driver:45", "cluster"), "cluster:45");
        UNIT_ASSERT_STRINGS_EQUAL(ApplyClusterEndpoint("driver:75", "cluster:8A0"), "[cluster:8A0]:75");
    }
}
