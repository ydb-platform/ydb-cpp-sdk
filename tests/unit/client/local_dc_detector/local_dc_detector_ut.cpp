#include <src/api/protos/ydb_discovery.pb.h>

#define INCLUDE_YDB_INTERNAL_H
#include <src/client/impl/internal/local_dc_detector/local_dc_detector.h>
#undef INCLUDE_YDB_INTERNAL_H

#include <util/network/sock.h>

#include <gtest/gtest.h>

using namespace NYdb;

class TMockedPinger : public IPinger {
public:
    void Reset() override {}

    void AddEndpoint(const Ydb::Discovery::EndpointInfo& endpoint, const std::chrono::seconds timeout) override {
        EndpointsPerLocation_[endpoint.location()].insert(endpoint.address());
    }

    NThreading::TFuture<std::string> GetFastestLocation() override {
        return NThreading::MakeFuture<std::string>("");
    }

    const std::unordered_map<std::string, std::unordered_set<std::string>>& GetSelectedEndpoints() {
        return EndpointsPerLocation_;
    }

private:
    std::unordered_map<std::string, std::unordered_set<std::string>> EndpointsPerLocation_;
};

struct LocalDCDetectionParams {
    std::vector<std::string> InputAddresses;
    
    size_t ExpectedLocationsAmount;
    std::unordered_map<std::string, size_t> ExpectedCountsPerLocation;
    
    std::string TestName; 
};

std::ostream& operator<<(std::ostream& os, const LocalDCDetectionParams& params) {
    return os << params.TestName;
}

class EndpointsSelectionTest : public ::testing::TestWithParam<LocalDCDetectionParams> {
protected:
    std::unique_ptr<TLocalDCDetector> detector;
    TMockedPinger* pinger;

    void SetUp() override {
        auto pingerPtr = std::make_unique<TMockedPinger>();
        pinger = pingerPtr.get();
        detector = std::make_unique<TLocalDCDetector>(std::move(pingerPtr));
    }
};

TEST_P(EndpointsSelectionTest, EndpointsSelection) {
    const auto& params = GetParam();

    Ydb::Discovery::ListEndpointsResult endpoints;
    for (const auto& ep : params.InputAddresses) {
        auto& endpoint = *endpoints.add_endpoints();
        endpoint.set_location(ep.substr(0, 1));
        endpoint.set_address(ep);
    }

    detector->DetectLocalDC(endpoints);

    auto selected = pinger->GetSelectedEndpoints();

    ASSERT_EQ(selected.size(), params.ExpectedLocationsAmount);

    for (const auto& [location, expectedCount] : params.ExpectedCountsPerLocation) {
        auto it = selected.find(location);
        
        ASSERT_NE(it, selected.end());
        EXPECT_EQ(it->second.size(), expectedCount);
    }

    for (const auto& [location, _] : selected) {
        EXPECT_TRUE(params.ExpectedCountsPerLocation.count(location));
    }
}

INSTANTIATE_TEST_SUITE_P(
    LocalDCDetectorTest,
    EndpointsSelectionTest,
    ::testing::Values (
        LocalDCDetectionParams{
            {"A1", "A2", "A3", "A4", "A5", "A6", "B1", "B2", "B3", "B4", "B5", "C1", "C2", "C3", "C4", "C5"},
            3,
            {{"A", 5}, {"B", 5}, {"C", 5}},
            "Basic"
        },
        LocalDCDetectionParams{
            {"A1", "A2", "A3", "B1"},
            2,
            {{"A", 3}, {"B", 1}},
            "BelowLimit"
        },
        LocalDCDetectionParams{
            {},
            0,
            {},
            "EmptyInput"
        },
        LocalDCDetectionParams{
            {"A1", "B1", "C1"},
            3,
            {{"A", 1}, {"B", 1}, {"C", 1}},
            "SingleNodesInLocations"
        }
    ) 
);

TEST (TLocalDCDetector, SingleLocation) {
    TLocalDCDetector detector;

    Ydb::Discovery::ListEndpointsResult endpoints;
    for (std::size_t i = 0; i < 3; ++i) {
        auto& endpoint = *endpoints.add_endpoints();
        endpoint.set_location("A");
        endpoint.set_address(std::to_string(i));
    }

    std::string localDC = detector.DetectLocalDC(endpoints);

    EXPECT_EQ(localDC, "");
}

ui16 GetAssignedPort(SOCKET s) {
    struct sockaddr_in6 bound_addr_sys;
    socklen_t addrLen = sizeof(bound_addr_sys);

    if (getsockname(s, (struct sockaddr*)&bound_addr_sys, &addrLen) == 0) {
        const ui16 assignedPort = ntohs(bound_addr_sys.sin6_port);
        
        return assignedPort;
    }
    return 0;
}

TEST (TPingerTest, FastestEndpoint) {
    TInetStreamSocket socket;
    {
        TSockAddrInet addr("127.0.0.1", 0);
        TBaseSocket::Check(socket.Bind(&addr));
        TBaseSocket::Check(socket.Listen(1));
    }

    Ydb::Discovery::EndpointInfo endpoint;
    endpoint.set_address("127.0.0.1");
    endpoint.set_port(GetAssignedPort(socket));
    endpoint.set_location("real");
    
    TInetStreamSocket fakeSocket;
    {
        TSockAddrInet addr("127.0.0.1", 0);
        TBaseSocket::Check(fakeSocket.Bind(&addr));
        TBaseSocket::Check(fakeSocket.Listen(1));
    }

    Ydb::Discovery::EndpointInfo fakeEndpoint;
    fakeEndpoint.set_address("127.0.0.1");
    fakeEndpoint.set_port(GetAssignedPort(fakeSocket));
    fakeEndpoint.set_location("fake");

    TPinger pinger;
    pinger.AddEndpoint(endpoint, std::chrono::seconds(1));
    pinger.AddEndpoint(fakeEndpoint, std::chrono::seconds(1));

    fakeSocket.Close();

    std::string localDC = pinger.GetFastestLocation().GetValue(std::chrono::seconds(1));
    pinger.Reset();

    EXPECT_EQ(localDC, "real");
}

TEST (TPingerTest, FailedPings) {
    TPinger pinger;
    for (std::size_t i = 0; i < 3; ++i) {
        TInetStreamSocket fakeSocket;
        {
            TSockAddrInet addr("127.0.0.1", 0);
            TBaseSocket::Check(fakeSocket.Bind(&addr));
            TBaseSocket::Check(fakeSocket.Listen(1));
        }
        Ydb::Discovery::EndpointInfo fakeEndpoint;
        fakeEndpoint.set_address("127.0.0.1");
        fakeEndpoint.set_port(GetAssignedPort(fakeSocket));
        fakeEndpoint.set_location("fake" + std::to_string(i));
        pinger.AddEndpoint(fakeEndpoint, std::chrono::seconds(1));
        fakeSocket.Close();
    }

    EXPECT_THROW(pinger.GetFastestLocation().GetValue(std::chrono::seconds(1)), std::runtime_error);
    pinger.Reset();
}
