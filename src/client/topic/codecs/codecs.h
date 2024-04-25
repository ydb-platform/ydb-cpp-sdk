#pragma once
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/topic/codecs/codecs.h
#include <ydb-cpp-sdk/util/stream/output.h>
#include <src/api/protos/ydb_topic.pb.h>
#include <ydb-cpp-sdk/client/topic/topic.h>
========
#include <src/util/stream/output.h>
#include <src/api/protos/ydb_topic.pb.h>
#include <src/client/ydb_topic/topic.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_topic/codecs/codecs.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_topic/codecs/codecs.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/stream/output.h>
#include <src/api/protos/ydb_topic.pb.h>
#include <ydb-cpp-sdk/client/topic/topic.h>
>>>>>>> origin/main


namespace NYdb::NTopic {
namespace NCompressionDetails {

extern std::string Decompress(const Ydb::Topic::StreamReadMessage::ReadResponse::MessageData& data, Ydb::Topic::Codec codec);

THolder<IOutputStream> CreateCoder(ECodec codec, TBuffer& result, int quality);

} // namespace NDecompressionDetails

} // namespace NYdb::NTopic
