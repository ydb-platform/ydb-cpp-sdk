#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/encode/encoder.h>
=======
#include <src/library/monlib/encode/encoder.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class IOutputStream;

namespace NMonitoring {
    IMetricEncoderPtr EncoderText(IOutputStream* out, bool humanReadableTs = true);
}
