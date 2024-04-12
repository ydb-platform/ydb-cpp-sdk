#pragma once

#include <ydb-cpp-sdk/library/monlib/dynamic_counters/counters.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/encode/encoder.h>
#include <ydb-cpp-sdk/library/monlib/encode/format.h>
=======
#include <src/library/monlib/encode/encoder.h>
#include <src/library/monlib/encode/format.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NMonitoring {

    THolder<ICountableConsumer> CreateEncoder(
        IOutputStream* out,
        EFormat format,
        std::string_view nameLabel = "sensor",
        TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public
    );

    THolder<ICountableConsumer> AsCountableConsumer(
        NMonitoring::IMetricEncoderPtr encoder,
        TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

    void ToJson(const TDynamicCounters& counters, IOutputStream* out);

    std::string ToJson(const TDynamicCounters& counters);
}
