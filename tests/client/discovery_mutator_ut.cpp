<<<<<<< HEAD
#include <ydb-cpp-sdk/client/extensions/discovery_mutator/discovery_mutator.h>

#include <src/api/grpc/ydb_discovery_v1.grpc.pb.h>
#include <ydb-cpp-sdk/client/ydb_extension/extension.h>
#include <ydb-cpp-sdk/client/table/table.h>
=======
#include <discovery_mutator.h>

#include <src/api/grpc/ydb_discovery_v1.grpc.pb.h>
#include <src/client/ydb_extension/extension.h>
#include <src/client/ydb_table/table.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <src/library/testing/unittest/registar.h>
#include <src/library/testing/unittest/tests_data.h>

using namespace NYdb;
using namespace NDiscoveryMutator;

Y_UNIT_TEST_SUITE(DiscoveryMutator) {
    Y_UNIT_TEST(Simple) {

        std::unordered_set<std::string_view> dbs;
        std::string discoveryEndpont = "localhost:100";

        auto mutator = [&](Ydb::Discovery::ListEndpointsResult* proto, TStatus status, const IDiscoveryMutatorApi::TAuxInfo& aux) {
            UNIT_ASSERT_VALUES_EQUAL(discoveryEndpont, status.GetEndpoint());
            UNIT_ASSERT_VALUES_EQUAL(discoveryEndpont, aux.DiscoveryEndpoint);
            dbs.insert(aux.Database);
            Y_UNUSED(proto);
            return status;
        };
        auto driver = TDriver(
            TDriverConfig()
                .SetDatabase("db1")
                .SetEndpoint(discoveryEndpont));

        driver.AddExtension<TDiscoveryMutator>(TDiscoveryMutator::TParams(std::move(mutator)));

        auto clientSettings = NTable::TClientSettings();
        clientSettings.Database("db2");

        // By default this is sync operation
        auto client = NTable::TTableClient(driver, clientSettings);
        UNIT_ASSERT(dbs.find("db2") != dbs.end());
    }
}
