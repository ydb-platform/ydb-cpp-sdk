#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/encode/encoder.h>
#include <ydb-cpp-sdk/library/monlib/encode/format.h>

#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/library/monlib/encode/encoder.h>
#include <src/library/monlib/encode/format.h>

#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))


namespace NMonitoring {

    class TPrometheusDecodeException: public yexception {
    };

    IMetricEncoderPtr EncoderPrometheus(IOutputStream* out, std::string_view metricNameLabel = "sensor");

    void DecodePrometheus(std::string_view data, IMetricConsumer* c, std::string_view metricNameLabel = "sensor");

}
