#pragma once

#include <ydb-cpp-sdk/library/monlib/encode/encoder.h>
#include <ydb-cpp-sdk/library/monlib/encode/format.h>

#include <ydb-cpp-sdk/util/generic/yexception.h>


namespace NMonitoring {

    class TPrometheusDecodeException: public yexception {
    };

    IMetricEncoderPtr EncoderPrometheus(IOutputStream* out, std::string_view metricNameLabel = "sensor");

    void DecodePrometheus(std::string_view data, IMetricConsumer* c, std::string_view metricNameLabel = "sensor");

}
