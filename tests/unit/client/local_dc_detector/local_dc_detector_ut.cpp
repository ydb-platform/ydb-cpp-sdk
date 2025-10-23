#include <src/api/protos/ydb_discovery.pb.h>

#define INCLUDE_YDB_INTERNAL_H
#include <src/client/impl/internal/local_dc_detector/local_dc_detector.h>
#undef INCLUDE_YDB_INTERNAL_H

#include <library/cpp/testing/unittest/registar.h>
#include <util/datetime/base.h>

using namespace NYdb;

class TMockedEndpoint {
public:
    explicit TMockedEndpoint(std::vector<TDuration> measures) 
        : Measures_(std::move(measures))
        , Idx_(0)
    {}

    TDuration Ping() {
        std::size_t idx = Idx_++;

        if (idx < Measures_.size()) {
            return Measures_.at(idx);
        }
        return TDuration::Max();
    }

private:
    const std::vector<TDuration> Measures_;
    std::size_t Idx_;
};

class TMockedPinger {
public:
    explicit TMockedPinger(std::unordered_map<std::string, std::vector<TDuration>> measuresByAdress) {
        EndpointByAdress_.reserve(measuresByAdress.size());
        
        for (auto& [adress, measures] : measuresByAdress) {
            EndpointByAdress_.emplace(std::move(adress), std::move(measures));
        }
    }

    TDuration operator()(const Ydb::Discovery::EndpointInfo& endpoint) const {
        auto it = EndpointByAdress_.find(endpoint.address());
        if (it == EndpointByAdress_.end() || Blacklist_.contains(endpoint.address())) {
            return TDuration::Max();
        }
        return it->second.Ping();
    }

    void BanEndpoint(const std::string& adress) {
        Blacklist_.insert(adress);
    }

    void UnbanEndpoint(const std::string& adress) {
        Blacklist_.erase(adress);
    }

private:
    mutable std::unordered_map<std::string, TMockedEndpoint> EndpointByAdress_;
    std::unordered_set<std::string> Blacklist_;
};

std::vector<TDuration> GenerateMeasures(size_t count, int minMs, int maxMs, std::mt19937& gen) {
    std::vector<TDuration> measures;
    measures.reserve(count);
    std::uniform_int_distribution<int> distrib(minMs, maxMs);
    for (size_t i = 0; i < count; ++i) {
        measures.push_back(TDuration::MicroSeconds(distrib(gen)));
    }
    return measures;
}

Y_UNIT_TEST_SUITE(LocalDCDetectionTest) {
    Y_UNIT_TEST(Basic) {
        Ydb::Discovery::ListEndpointsResult endpoints;
        std::unordered_map<std::string, std::vector<TDuration>> mockData;
        std::mt19937 gen(std::random_device{}());

        const std::vector<std::string> endpointsA = {"A1", "A2", "A3", "A4", "A5", "A6", "A7"};
        const std::vector<std::string> endpointsB = {"B1", "B2", "B3"};
        const std::vector<std::string> endpointsC = {"C1", "C2", "C3", "C4", "C5", "C6", "C7"};

        const std::size_t epoches = 3;
        const std::size_t measuresAmount = 10 * epoches;

        for (const auto& ep : endpointsA) {
            mockData[ep] = GenerateMeasures(measuresAmount, 20, 30, gen);
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("A");
            endpoint.set_address(ep);
        }
        for (const auto& ep : endpointsB) {
            mockData[ep] = GenerateMeasures(measuresAmount, 30, 45, gen);
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("B");
            endpoint.set_address(ep);
        }
        for (const auto& ep : endpointsC) {
            mockData[ep] = GenerateMeasures(measuresAmount, 50, 70, gen);
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("C");
            endpoint.set_address(ep);
        }

        std::function<TDuration(const Ydb::Discovery::EndpointInfo& endpoint)> pinger = TMockedPinger(mockData);
        TLocalDCDetector detector(pinger);

        for (std::size_t i = 0; i < epoches; ++i) {
            detector.DetectLocalDC(endpoints);
            UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("A"));
            UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("B"));
            UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("C"));
        }
    }

    Y_UNIT_TEST(SingleLocation) {
        Ydb::Discovery::ListEndpointsResult endpoints;
        std::unordered_map<std::string, std::vector<TDuration>> mockData;
        std::mt19937 gen(std::random_device{}());

        const std::vector<std::string> endpointsA = {"A1", "A2", "A3"};

        for (const auto& ep : endpointsA) {
            mockData[ep] = GenerateMeasures(10, 20, 30, gen);
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("A");
            endpoint.set_address(ep);
        }

        std::function<TDuration(const Ydb::Discovery::EndpointInfo& endpoint)> pinger = TMockedPinger(mockData);
        TLocalDCDetector detector(pinger);

        detector.DetectLocalDC(endpoints);

        UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("A"));
    }

    Y_UNIT_TEST(UnavailableLocalDC) {
        Ydb::Discovery::ListEndpointsResult endpoints;
        std::unordered_map<std::string, std::vector<TDuration>> mockData;
        std::mt19937 gen(std::random_device{}());

        const std::vector<std::string> endpointsA = {"A1", "A2", "A3", "A4", "A5", "A6", "A7"};
        const std::vector<std::string> endpointsB = {"B1", "B2", "B3", "B4", "B5", "B6", "B7"};
        const std::vector<std::string> endpointsC = {"C1", "C2", "C3", "C4", "C5", "C6", "C7"};

        const std::size_t epoches = 3;
        const std::size_t measuresAmount = 10 * epoches;

        for (const auto& ep : endpointsA) {
            mockData[ep] = GenerateMeasures(measuresAmount, 20, 30, gen);
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("A");
            endpoint.set_address(ep);
        }
        for (const auto& ep : endpointsB) {
            mockData[ep] = GenerateMeasures(measuresAmount, 30, 45, gen);
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("B");
            endpoint.set_address(ep);
        }
        for (const auto& ep : endpointsC) {
            mockData[ep] = GenerateMeasures(measuresAmount, 50, 70, gen);
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("C");
            endpoint.set_address(ep);
        }

        TMockedPinger mockPinger(mockData);
        std::function<TDuration(const Ydb::Discovery::EndpointInfo& endpoint)> pinger = 
            [&mockPinger](const Ydb::Discovery::EndpointInfo& endpoint) { return mockPinger(endpoint); };

        TLocalDCDetector detector(pinger);

        detector.DetectLocalDC(endpoints);
        UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("A"));
        UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("B"));
        UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("C"));

        for (const auto& ep : endpointsA) {
            mockPinger.BanEndpoint(ep);
        }

        detector.DetectLocalDC(endpoints);
        UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("A"));
        UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("B"));
        UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("C"));

        for (const auto& ep : endpointsA) {
            mockPinger.UnbanEndpoint(ep);
        }

        detector.DetectLocalDC(endpoints);
        UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("A"));
        UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("B"));
        UNIT_ASSERT_VALUES_EQUAL(false, detector.IsLocalDC("C"));
    }

    Y_UNIT_TEST(OfflineDCs) {
        Ydb::Discovery::ListEndpointsResult endpoints;
        std::unordered_map<std::string, std::vector<TDuration>> mockData;
        std::mt19937 gen(std::random_device{}());

        const std::vector<std::string> endpointsA = {"A1", "A2", "A3", "A4", "A5", "A6", "A7"};
        const std::vector<std::string> endpointsB = {"B1", "B2", "B3", "B4", "B5", "B6", "B7"};
        const std::vector<std::string> endpointsC = {"C1", "C2", "C3", "C4", "C5", "C6", "C7"};

        for (const auto& ep : endpointsA) {
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("A");
            endpoint.set_address(ep);
        }
        for (const auto& ep : endpointsB) {
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("B");
            endpoint.set_address(ep);
        }
        for (const auto& ep : endpointsC) {
            auto& endpoint = *endpoints.add_endpoints();
            endpoint.set_location("C");
            endpoint.set_address(ep);
        }

        TMockedPinger mockPinger(mockData);
        std::function<TDuration(const Ydb::Discovery::EndpointInfo& endpoint)> pinger = 
            [&mockPinger](const Ydb::Discovery::EndpointInfo& endpoint) { return mockPinger(endpoint); };

        TLocalDCDetector detector(pinger);

        for (const auto& ep : endpointsA) {
            mockPinger.BanEndpoint(ep);
        }
        for (const auto& ep : endpointsB) {
            mockPinger.BanEndpoint(ep);
        }
        for (const auto& ep : endpointsC) {
            mockPinger.BanEndpoint(ep);
        }

        detector.DetectLocalDC(endpoints);
        UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("A"));
        UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("B"));
        UNIT_ASSERT_VALUES_EQUAL(true, detector.IsLocalDC("C"));
    }
}
