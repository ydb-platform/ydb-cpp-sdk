#pragma once

#include <src/library/monlib/encode/encoder.h>
#include <src/library/monlib/encode/format.h>


class IOutputStream;

namespace NMonitoring {

    class TJsonDecodeError: public yexception {
    };

    IMetricEncoderPtr EncoderJson(IOutputStream* out, int indentation = 0);

    /// Buffered encoder will merge series with same labels into one.
    IMetricEncoderPtr BufferedEncoderJson(IOutputStream* out, int indentation = 0);

    IMetricEncoderPtr EncoderCloudJson(IOutputStream* out,
                                       int indentation = 0,
                                       std::string_view metricNameLabel = "name");

    IMetricEncoderPtr BufferedEncoderCloudJson(IOutputStream* out,
                                               int indentation = 0,
                                               std::string_view metricNameLabel = "name");

    void DecodeJson(std::string_view data, IMetricConsumer* c, std::string_view metricNameLabel = "name");

}
