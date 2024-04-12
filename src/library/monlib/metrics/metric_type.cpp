#include <ydb-cpp-sdk/library/monlib/metrics/metric_type.h>

#include <string_view>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/generic/yexception.h>
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NMonitoring {
    std::string_view MetricTypeToStr(EMetricType type) {
        switch (type) {
            case EMetricType::GAUGE:
                return std::string_view("GAUGE");
            case EMetricType::COUNTER:
                return std::string_view("COUNTER");
            case EMetricType::RATE:
                return std::string_view("RATE");
            case EMetricType::IGAUGE:
                return std::string_view("IGAUGE");
            case EMetricType::HIST:
                return std::string_view("HIST");
            case EMetricType::HIST_RATE:
                return std::string_view("HIST_RATE");
            case EMetricType::DSUMMARY:
                return std::string_view("DSUMMARY");
            case EMetricType::LOGHIST:
                return std::string_view("LOGHIST");
            default:
                return std::string_view("UNKNOWN");
        }
    }

    EMetricType MetricTypeFromStr(std::string_view str) {
        if (str == std::string_view("GAUGE") || str == std::string_view("DGAUGE")) {
            return EMetricType::GAUGE;
        } else if (str == std::string_view("COUNTER")) {
            return EMetricType::COUNTER;
        } else if (str == std::string_view("RATE")) {
            return EMetricType::RATE;
        } else if (str == std::string_view("IGAUGE")) {
            return EMetricType::IGAUGE;
        } else if (str == std::string_view("HIST")) {
            return EMetricType::HIST;
        } else if (str == std::string_view("HIST_RATE")) {
            return EMetricType::HIST_RATE;
        } else if (str == std::string_view("DSUMMARY")) {
            return EMetricType::DSUMMARY;
        } else if (str == std::string_view("LOGHIST")) {
            return EMetricType::LOGHIST;
        } else {
            ythrow yexception() << "unknown metric type: " << str;
        }
    }
}

template <>
void Out<NMonitoring::EMetricType>(IOutputStream& o, NMonitoring::EMetricType t) {
    o << NMonitoring::MetricTypeToStr(t);
}
