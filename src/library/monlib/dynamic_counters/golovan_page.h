#pragma once

#include <ydb-cpp-sdk/library/monlib/dynamic_counters/counters.h>

#include <ydb-cpp-sdk/library/monlib/service/pages/mon_page.h>

#include <src/util/generic/ptr.h>

#include <functional>

// helper class to output json for Golovan.
class TGolovanCountersPage: public NMonitoring::IMonPage {
public:
    using TOutputCallback = std::function<void()>;

    const TIntrusivePtr<NMonitoring::TDynamicCounters> Counters;

    TGolovanCountersPage(const std::string& path, TIntrusivePtr<NMonitoring::TDynamicCounters> counters,
                         TOutputCallback outputCallback = nullptr);

    void Output(NMonitoring::IMonHttpRequest& request) override;

private:
    TOutputCallback OutputCallback;
};
