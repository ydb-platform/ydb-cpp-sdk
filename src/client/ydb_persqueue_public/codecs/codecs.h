#pragma once
<<<<<<<< HEAD:src/client/persqueue_public/codecs/codecs.h
#include <ydb-cpp-sdk/util/stream/output.h>
#include <src/api/protos/ydb_persqueue_v1.pb.h>
#include <src/client/persqueue_core/persqueue.h>
#include <ydb-cpp-sdk/client/topic/topic.h>
========
#include <src/util/stream/output.h>
#include <src/api/protos/ydb_persqueue_v1.pb.h>
#include <src/client/ydb_persqueue_core/persqueue.h>
#include <src/client/ydb_topic/topic.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_persqueue_public/codecs/codecs.h


namespace NYdb::NPersQueue {
namespace NCompressionDetails {

extern std::string Decompress(const Ydb::PersQueue::V1::MigrationStreamingReadServerMessage::DataBatch::MessageData& data);

THolder<IOutputStream> CreateCoder(ECodec codec, TBuffer& result, int quality);

} // namespace NDecompressionDetails

} // namespace NYdb::NPersQueue
