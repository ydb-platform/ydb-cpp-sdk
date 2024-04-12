#pragma once

#include <ydb-cpp-sdk/library/monlib/dynamic_counters/counters.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/service/pages/mon_page.h>

#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/library/monlib/service/pages/mon_page.h>

#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
