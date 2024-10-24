#include <ydb-cpp-sdk/client/draft/ydb_scripting.h>
#include <ydb-cpp-sdk/type_switcher.h>

#include <src/api/grpc/ydb_table_v1.grpc.pb.h>
#include <src/api/grpc/ydb_scripting_v1.grpc.pb.h>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <library/cpp/testing/unittest/registar.h>
#include <library/cpp/testing/unittest/tests_data.h>

using namespace NYdb;
using namespace NYdb::NScripting;

namespace {

template<class TService>
std::unique_ptr<grpc::Server> StartGrpcServer(const std::string& address, TService& service) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(TStringType{address}, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    return builder.BuildAndStart();
}

class TMockSlyDbProxy : public Ydb::Scripting::V1::ScriptingService::Service
{
public:
    grpc::Status ExecuteYql(
        grpc::ServerContext* context,
        const Ydb::Scripting::ExecuteYqlRequest* request,
        Ydb::Scripting::ExecuteYqlResponse* response) override {
            context->AddInitialMetadata("key", "value");
            Y_UNUSED(request);

            // Just to make sdk core happy
            auto* op = response->mutable_operation();
            op->set_ready(true);
            op->set_status(Ydb::StatusIds::SUCCESS);
            op->mutable_result();

            return grpc::Status::OK;
        }
};

}

Y_UNIT_TEST_SUITE(ResponseHeaders) {
    Y_UNIT_TEST(PassHeader) {
        TMockSlyDbProxy slyDbProxy;

        std::string addr = "localhost:10000";

        auto server = StartGrpcServer(addr, slyDbProxy);

        auto config = TDriverConfig()
            .SetEndpoint(addr);
        TDriver driver(config);
        TScriptingClient client(driver);

        auto result = client.ExecuteYqlScript("SMTH").GetValueSync();
        auto metadata = result.GetResponseMetadata();

        UNIT_ASSERT(metadata.find("key") != metadata.end());
        UNIT_ASSERT_VALUES_EQUAL(metadata.find("key")->second, "value");
    }
}
