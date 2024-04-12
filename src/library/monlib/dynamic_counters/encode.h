#pragma once

#include "counters.h"

#include <src/library/monlib/encode/encoder.h>
#include <src/library/monlib/encode/format.h>

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
