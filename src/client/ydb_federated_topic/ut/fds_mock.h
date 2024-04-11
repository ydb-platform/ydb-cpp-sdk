#pragma once

#include <library/cpp/threading/future/future.h>
#include <ydb/public/api/grpc/ydb_federation_discovery_v1.grpc.pb.h>

#include <deque>
#include <optional>

namespace NYdb::NFederatedTopic::NTests {

// ctor gets ---list of response--- optional{response}
// ListFederationDatabases gets 1 element under lock and respond. otherwise
// create 2 queues, for requests and responses
// getrequest() - put request and returns<request, promise<response>>
// sendresponse()
class TFederationDiscoveryServiceMock: public Ydb::FederationDiscovery::V1::FederationDiscoveryService::Service {
public:
    using TRequest = Ydb::FederationDiscovery::ListFederationDatabasesRequest;
    using TResponse = Ydb::FederationDiscovery::ListFederationDatabasesResponse;

    struct TGrpcResult {
        TResponse Response;
        grpc::Status Status;
    };

    struct TManualRequest {
        const TRequest* Request;
        NThreading::TPromise<TGrpcResult> Result;
    };

public:
    ~TFederationDiscoveryServiceMock() {
        while (auto r = GetNextPendingRequest()) {
            r->Result.SetValue({});
        }
    }

    std::optional<TManualRequest> GetNextPendingRequest() {
        std::optional<TManualRequest> result;
        std::lock_guard guard(Lock);
        if (!PendingRequests.empty()) {
            result = PendingRequests.front();
            PendingRequests.pop_front();
        }
        return result;
    }

    virtual grpc::Status ListFederationDatabases(grpc::ServerContext*,
                              const TRequest* request,
                              TResponse* response) override {
        Y_UNUSED(request);

        auto p = NThreading::NewPromise<TGrpcResult>();
        auto f = p.GetFuture();

        {
            std::lock_guard guard(Lock);
            PendingRequests.push_back({request, std::move(p)});
        }

        f.Wait(TDuration::Seconds(35));
        if (f.HasValue()) {
            auto result = f.ExtractValueSync();
            std::cerr << ">>> Ready to answer: " << (result.Status.ok() ? "ok" : "err") << std::endl;
            *response = std::move(result.Response);
            return result.Status;
        }

        return grpc::Status(grpc::StatusCode::UNKNOWN, "No response after timeout");
    }

    TGrpcResult ComposeOkResult() {
        Ydb::FederationDiscovery::ListFederationDatabasesResponse okResponse;

        auto op = okResponse.mutable_operation();
        op->set_status(Ydb::StatusIds::SUCCESS);
        okResponse.mutable_operation()->set_ready(true);
        okResponse.mutable_operation()->set_id("12345");

        Ydb::FederationDiscovery::ListFederationDatabasesResult mockResult;
        mockResult.set_control_plane_endpoint("cp.logbroker-federation:2135");
        mockResult.set_self_location("fancy_datacenter");
        auto c1 = mockResult.add_federation_databases();
        c1->set_name("dc1");
        c1->set_path("/Root");
        c1->set_id("account-dc1");
        c1->set_endpoint("localhost:" + ToString(Port));
        c1->set_location("dc1");
        c1->set_status(::Ydb::FederationDiscovery::DatabaseInfo::Status::DatabaseInfo_Status_AVAILABLE);
        c1->set_weight(1000);
        auto c2 = mockResult.add_federation_databases();
        c2->set_name("dc2");
        c2->set_path("/Root");
        c2->set_id("account-dc2");
        c2->set_endpoint("localhost:" + ToString(Port));
        c2->set_location("dc2");
        c2->set_status(::Ydb::FederationDiscovery::DatabaseInfo::Status::DatabaseInfo_Status_AVAILABLE);
        c2->set_weight(500);

        op->mutable_result()->PackFrom(mockResult);

        return {okResponse, grpc::Status::OK};
    }


public:
    ui16 Port;
    std::deque<TManualRequest> PendingRequests;
    TAdaptiveLock Lock;
};

}  // namespace NYdb::NFederatedTopic::NTests
