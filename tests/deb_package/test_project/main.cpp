#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/iam/iam.h>
#include <google/api/http.pb.h>

#include <iostream>

int main() {
    auto config = NYdb::TDriverConfig()
        .SetEndpoint("localhost:2136")
        .SetDatabase("/local")
        .SetCredentialsProviderFactory(NYdb::CreateIamCredentialsProviderFactory({}));

    std::cout << "Successfully instantiated driver configuration with IAM credentials provider." << std::endl;
    return 0;
}