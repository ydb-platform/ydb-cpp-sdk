<<<<<<< HEAD
<<<<<<<< HEAD:src/client/topic/impl/counters.cpp
#include <ydb-cpp-sdk/client/topic/topic.h>
========
#include <src/client/ydb_topic/topic.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_topic/impl/counters.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_topic/impl/counters.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/topic/topic.h>
>>>>>>> origin/main

namespace NYdb::NTopic {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NTopic::TReaderCounters

TReaderCounters::TReaderCounters(const TIntrusivePtr<::NMonitoring::TDynamicCounters>& counters) {
    Errors = counters->GetCounter("errors", true);
    CurrentSessionLifetimeMs = counters->GetCounter("currentSessionLifetimeMs", false);
    BytesRead = counters->GetCounter("bytesRead", true);
    MessagesRead = counters->GetCounter("messagesRead", true);
    BytesReadCompressed = counters->GetCounter("bytesReadCompressed", true);
    BytesInflightUncompressed = counters->GetCounter("bytesInflightUncompressed", false);
    BytesInflightCompressed = counters->GetCounter("bytesInflightCompressed", false);
    BytesInflightTotal = counters->GetCounter("bytesInflightTotal", false);
    MessagesInflight = counters->GetCounter("messagesInflight", false);

#define HISTOGRAM_SETUP NMonitoring::ExplicitHistogram({0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100})

    TotalBytesInflightUsageByTime = counters->GetHistogram("totalBytesInflightUsageByTime", HISTOGRAM_SETUP);
    UncompressedBytesInflightUsageByTime = counters->GetHistogram("uncompressedBytesInflightUsageByTime", HISTOGRAM_SETUP);
    CompressedBytesInflightUsageByTime = counters->GetHistogram("compressedBytesInflightUsageByTime", HISTOGRAM_SETUP);

#undef HISTOGRAM_SETUP

}

}
