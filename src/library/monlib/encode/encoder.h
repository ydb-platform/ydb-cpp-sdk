#pragma once

#include <util/generic/ptr.h>

#include <src/library/monlib/metrics/metric_consumer.h>

namespace NMonitoring {
    class IMetricEncoder: public IMetricConsumer {
    public:
        virtual ~IMetricEncoder();

        virtual void Close() = 0;
    };

    using IMetricEncoderPtr = THolder<IMetricEncoder>;

}
