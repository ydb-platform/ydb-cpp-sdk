#pragma once

#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_consumer.h>

namespace NMonitoring {
    class IMetricEncoder: public IMetricConsumer {
    public:
        virtual ~IMetricEncoder();

        virtual void Close() = 0;
    };

    using IMetricEncoderPtr = THolder<IMetricEncoder>;

}
