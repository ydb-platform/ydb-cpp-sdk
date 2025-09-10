#pragma once

#include <ydb-cpp-sdk/client/topic/codecs.h>
#include <ydb-cpp-sdk/client/topic/errors.h>
#include <ydb-cpp-sdk/client/topic/events_common.h>
#include <ydb-cpp-sdk/client/topic/executor.h>
#include <ydb-cpp-sdk/client/topic/retry_policy.h>

#ifndef YDB_TOPIC_DISABLE_COUNTERS
#include <ydb-cpp-sdk/client/topic/counters.h>
#endif

namespace NYdb::inline V3::NPersQueue {

// codecs
using NTopic::ECodec;
using NTopic::ICodec;
using NTopic::TCodecMap;
using NTopic::TGzipCodec;
using NTopic::TZstdCodec;
using NTopic::TUnsupportedCodec;

#ifndef YDB_TOPIC_DISABLE_COUNTERS
// counters
using NTopic::TCounterPtr;
using NTopic::TReaderCounters;
using NTopic::TWriterCounters;
using NTopic::MakeCountersNotNull;
using NTopic::HasNullCounters;
#endif

// errors
// using NTopic::GetRetryErrorClass;
// using NTopic::GetRetryErrorClassV2;

// common events
using NTopic::TWriteSessionMeta;
using NTopic::TMessageMeta;
using NTopic::TSessionClosedEvent;
using NTopic::TSessionClosedHandler;
// TODO reuse TPrintable

// executor
using NTopic::IExecutor;
using NTopic::CreateThreadPoolExecutorAdapter;
using NTopic::CreateThreadPoolExecutor;
using NTopic::CreateSyncExecutor;

// retry policy
using NTopic::IRetryPolicy;

}  // namespace NYdb::NPersQueue
