#pragma once

#include <src/library/monlib/encode/encoder.h>
#include <src/library/monlib/encode/format.h>

#include <src/util/generic/yexception.h>


namespace NMonitoring {

    class TPrometheusDecodeException: public yexception {
    };

    IMetricEncoderPtr EncoderPrometheus(IOutputStream* out, std::string_view metricNameLabel = "sensor");

    void DecodePrometheus(std::string_view data, IMetricConsumer* c, std::string_view metricNameLabel = "sensor");

}
