#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/monlib/metrics/metric_type.h
#include <ydb-cpp-sdk/util/generic/fwd.h>
========
#include <src/util/generic/fwd.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/monlib/metrics/metric_type.h

#include <string_view>

namespace NMonitoring {

    constexpr ui32 MaxMetricTypeNameLength = 9;

    enum class EMetricType {
        UNKNOWN = 0,
        GAUGE = 1,
        COUNTER = 2,
        RATE = 3,
        IGAUGE = 4,
        HIST = 5,
        HIST_RATE = 6,
        DSUMMARY = 7,
        // ISUMMARY = 8, reserved
        LOGHIST = 9,
    };

    std::string_view MetricTypeToStr(EMetricType type);
    EMetricType MetricTypeFromStr(std::string_view str);

}
