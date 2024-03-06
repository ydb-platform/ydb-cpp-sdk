#pragma once



#include <library/cpp/monlib/metrics/metric_consumer.h>

namespace NMonitoring {
    class IMetricEncoder: public IMetricConsumer {
    public:
        virtual ~IMetricEncoder();

        virtual void Close() = 0;
    };

    using IMetricEncoderPtr = std::unique_ptr<IMetricEncoder>;

}
