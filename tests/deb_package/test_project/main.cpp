#include <ydb-cpp-sdk/client/coordination/distributed_lock.h>
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/iam/iam.h>
#include <ydb-cpp-sdk/client/topic/producer.h>
#include <google/api/http.pb.h>

#include <iostream>

int main() {
    auto config = NYdb::TDriverConfig()
        .SetEndpoint("localhost:2136")
        .SetDatabase("/local")
        .SetCredentialsProviderFactory(NYdb::CreateIamCredentialsProviderFactory({}));

    NYdb::NTopic::TProducerSettings producerSettings;
    if (producerSettings.PartitioningKeyHasher_("package-test").size() != 8) {
        std::cerr << "Unexpected partitioning key hash size." << std::endl;
        return 1;
    }

    try {
        NYdb::NCoordination::TDistributedLock lock({}, {});
        std::cerr << "Distributed lock accepted an empty session." << std::endl;
        return 1;
    } catch (const NYdb::NCoordination::TYdbLockException&) {
    }

    std::cout << "Successfully checked driver configuration, IAM, producer settings and distributed locks." << std::endl;
    return 0;
}
