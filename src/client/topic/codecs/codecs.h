#pragma once
#include <src/util/stream/output.h>
#include <src/api/protos/ydb_topic.pb.h>
#include <ydb-cpp-sdk/client/topic/topic.h>


namespace NYdb::NTopic {
namespace NCompressionDetails {

extern std::string Decompress(const Ydb::Topic::StreamReadMessage::ReadResponse::MessageData& data, Ydb::Topic::Codec codec);

THolder<IOutputStream> CreateCoder(ECodec codec, TBuffer& result, int quality);

} // namespace NDecompressionDetails

} // namespace NYdb::NTopic
