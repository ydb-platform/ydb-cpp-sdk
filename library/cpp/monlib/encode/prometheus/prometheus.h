#pragma once

#include <library/cpp/monlib/encode/encoder.h>
#include <library/cpp/monlib/encode/format.h>

#include <util/generic/yexception.h>


namespace NMonitoring {

    class TPrometheusDecodeException: public yexception {
    };

    IMetricEncoderPtr EncoderPrometheus(IOutputStream* out, std::string_view metricNameLabel = "sensor");

    void DecodePrometheus(std::string_view data, IMetricConsumer* c, std::string_view metricNameLabel = "sensor");

}
