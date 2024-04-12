#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/monlib/encode/encoder.h
#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_consumer.h>
========
#include <src/util/generic/ptr.h>

#include <src/library/monlib/metrics/metric_consumer.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/monlib/encode/encoder.h

namespace NMonitoring {
    class IMetricEncoder: public IMetricConsumer {
    public:
        virtual ~IMetricEncoder();

        virtual void Close() = 0;
    };

    using IMetricEncoderPtr = THolder<IMetricEncoder>;

}
