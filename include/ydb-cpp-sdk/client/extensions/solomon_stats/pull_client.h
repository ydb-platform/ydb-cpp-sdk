#pragma once

#include <ydb-cpp-sdk/client/ydb_extension/extension.h>

#include <ydb-cpp-sdk/library/http/server/response.h>
#include <src/library/monlib/metrics/metric_consumer.h>
#include <src/library/monlib/encode/json/json.h>
#include <src/library/monlib/metrics/metric_registry.h>
#include <src/library/monlib/service/pages/mon_page.h>
#include <src/library/monlib/service/monservice.h>

namespace NSolomonStatExtension {

class TSolomonStatPullExtension: public NYdb::IExtension {
public:
    using IApi = NYdb::NSdkStats::IStatApi;

    class TParams {
        friend class TSolomonStatPullExtension;

    public:
        TParams(const std::string& host
                , ui16 port
                , const std::string& project
                , const std::string& service
                , const std::string& cluster
                , const std::vector<std::pair<std::string, std::string>>& labels = {});

        NMonitoring::TLabels GetLabels() const;

    private:
        const std::string Host_;
        ui16 Port_;
        NMonitoring::TLabels Labels_;
    };

    TSolomonStatPullExtension(const TParams& params, IApi* api);
    ~TSolomonStatPullExtension();

private:
    class TSolomonStatPage: public NMonitoring::IMonPage {
        friend class TSolomonStatPullExtension;
    public:
        TSolomonStatPage(const std::string& title, const std::string& path, IApi* api);

        void Output(NMonitoring::IMonHttpRequest& request) override ;

    private:
        IApi* Api_;
    };

private:
    std::shared_ptr<NMonitoring::TMetricRegistry> MetricRegistry_;
    NMonitoring::TMonService2 MonService_;
    TIntrusivePtr<TSolomonStatPage> Page_;
};

} // namespace NSolomonStatExtension
