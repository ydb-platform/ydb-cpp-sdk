#pragma once

#include <ydb-cpp-sdk/library/monlib/dynamic_counters/counters.h>

#include <src/library/monlib/service/pages/pre_mon_page.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <functional>

namespace NMonitoring {
    enum class EUnknownGroupPolicy {
        Error,  // send 404
        Ignore, // send 204
    };

    struct TDynamicCountersPage: public TPreMonPage {
    public:
        using TOutputCallback = std::function<void()>;

    private:
        const TIntrusivePtr<TDynamicCounters> Counters;
        TOutputCallback OutputCallback;
        EUnknownGroupPolicy UnknownGroupPolicy {EUnknownGroupPolicy::Error};

    private:
        void HandleAbsentSubgroup(IMonHttpRequest& request);

    public:
        TDynamicCountersPage(const std::string& path,
                             const std::string& title,
                             TIntrusivePtr<TDynamicCounters> counters,
                             TOutputCallback outputCallback = nullptr)
            : TPreMonPage(path, title)
            , Counters(counters)
            , OutputCallback(outputCallback)
        {
        }

        void Output(NMonitoring::IMonHttpRequest& request) override;

        void BeforePre(NMonitoring::IMonHttpRequest& request) override;

        void OutputText(IOutputStream& out, NMonitoring::IMonHttpRequest&) override;

        /// If set to Error, responds with 404 if the requested subgroup is not found. This is the default.
        /// If set to Ignore, responds with 204 if the requested subgroup is not found
        void SetUnknownGroupPolicy(EUnknownGroupPolicy value);
    };
}
