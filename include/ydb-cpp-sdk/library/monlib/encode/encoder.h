#pragma once

#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_consumer.h>

#include <memory>

namespace NMonitoring {
    class IMetricEncoder: public IMetricConsumer {
    public:
        virtual ~IMetricEncoder();

        virtual void Close() = 0;
    };

    using IMetricEncoderPtr = std::unique_ptr<IMetricEncoder>;

}
