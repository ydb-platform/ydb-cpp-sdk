#pragma once
#include <ydb-cpp-sdk/util/stream/output.h>
#include <src/api/protos/ydb_persqueue_v1.pb.h>
#include <src/client/persqueue_core/persqueue.h>
#include <ydb-cpp-sdk/client/topic/topic.h>


namespace NYdb::NPersQueue {
namespace NCompressionDetails {

extern std::string Decompress(const Ydb::PersQueue::V1::MigrationStreamingReadServerMessage::DataBatch::MessageData& data);

THolder<IOutputStream> CreateCoder(ECodec codec, TBuffer& result, int quality);

} // namespace NDecompressionDetails

} // namespace NYdb::NPersQueue
