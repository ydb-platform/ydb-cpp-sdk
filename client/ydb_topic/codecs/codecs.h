#pragma once
#include <util/stream/output.h>
#include <ydb/public/api/protos/ydb_topic.pb.h>
#include <client/ydb_topic/topic.h>


namespace NYdb::NTopic {
namespace NCompressionDetails {

extern std::string Decompress(const Ydb::Topic::StreamReadMessage::ReadResponse::MessageData& data, Ydb::Topic::Codec codec);

std::unique_ptr<IOutputStream> CreateCoder(ECodec codec, TBuffer& result, int quality);

} // namespace NDecompressionDetails

} // namespace NYdb::NTopic
