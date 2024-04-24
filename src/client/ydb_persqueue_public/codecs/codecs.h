#pragma once
#include <src/util/stream/output.h>
#include <src/api/protos/ydb_persqueue_v1.pb.h>
#include <src/client/ydb_persqueue_core/persqueue.h>
#include <src/client/ydb_topic/topic.h>


namespace NYdb::NPersQueue {
namespace NCompressionDetails {

extern std::string Decompress(const Ydb::PersQueue::V1::MigrationStreamingReadServerMessage::DataBatch::MessageData& data);

THolder<IOutputStream> CreateCoder(ECodec codec, TBuffer& result, int quality);

} // namespace NDecompressionDetails

} // namespace NYdb::NPersQueue
