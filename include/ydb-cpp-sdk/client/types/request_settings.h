#pragma once

#include "fluent_settings_helpers.h"

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/client/types/request_settings.h
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/request_settings.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/request_settings.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/datetime/base.h>
>>>>>>> origin/main

#include <vector>
#include <utility>

namespace NYdb {

template<typename TDerived>
struct TRequestSettings {
    using TSelf = TDerived;
    using THeader = std::vector<std::pair<std::string, std::string>>;

    FLUENT_SETTING(std::string, TraceId);
    FLUENT_SETTING(std::string, RequestType);
    FLUENT_SETTING(THeader, Header);
    FLUENT_SETTING(TDuration, ClientTimeout);

    TRequestSettings() = default;

    template <typename T>
    explicit TRequestSettings(const TRequestSettings<T>& other)
        : TraceId_(other.TraceId_)
        , RequestType_(other.RequestType_)
        , Header_(other.Header_)
        , ClientTimeout_(other.ClientTimeout_)
    {}
};

template<typename TDerived>
struct TSimpleRequestSettings : public TRequestSettings<TDerived> {
    using TSelf = TDerived;

    TSimpleRequestSettings() = default;

    template <typename T>
    explicit TSimpleRequestSettings(const TSimpleRequestSettings<T>& other)
        : TRequestSettings<TDerived>(other)
    {}
};

template<typename TDerived>
struct TOperationRequestSettings : public TSimpleRequestSettings<TDerived> {
    using TSelf = TDerived;

    /* Cancel/timeout operation settings available from 18-8 YDB server version */
    FLUENT_SETTING(TDuration, OperationTimeout);
    FLUENT_SETTING(TDuration, CancelAfter);
    FLUENT_SETTING(TDuration, ForgetAfter);
    FLUENT_SETTING_DEFAULT(bool, UseClientTimeoutForOperation, true);
    FLUENT_SETTING_DEFAULT(bool, ReportCostInfo, false);

    TOperationRequestSettings() = default;

    template <typename T>
    explicit TOperationRequestSettings(const TOperationRequestSettings<T>& other)
        : TSimpleRequestSettings<TDerived>(other)
        , OperationTimeout_(other.OperationTimeout_)
        , CancelAfter_(other.CancelAfter_)
        , ForgetAfter_(other.ForgetAfter_)
        , UseClientTimeoutForOperation_(other.UseClientTimeoutForOperation_)
        , ReportCostInfo_(other.ReportCostInfo_)
    {}

    TSelf& CancelAfterWithTimeout(const TDuration& cancelAfter, const TDuration& operationTimeout) {
        CancelAfter_ = cancelAfter;
        OperationTimeout_ = operationTimeout;
        return static_cast<TSelf&>(*this);
    }
};

} // namespace NYdb
