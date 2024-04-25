#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/monlib/encode/encoder.h
#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_consumer.h>
========
#include <src/util/generic/ptr.h>

#include <src/library/monlib/metrics/metric_consumer.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/monlib/encode/encoder.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/monlib/encode/encoder.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_consumer.h>
>>>>>>> origin/main

namespace NMonitoring {
    class IMetricEncoder: public IMetricConsumer {
    public:
        virtual ~IMetricEncoder();

        virtual void Close() = 0;
    };

    using IMetricEncoderPtr = THolder<IMetricEncoder>;

}
