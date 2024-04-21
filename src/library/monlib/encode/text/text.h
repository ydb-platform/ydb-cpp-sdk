#pragma once

#include <ydb-cpp-sdk/library/monlib/encode/encoder.h>

class IOutputStream;

namespace NMonitoring {
    IMetricEncoderPtr EncoderText(IOutputStream* out, bool humanReadableTs = true);
}
