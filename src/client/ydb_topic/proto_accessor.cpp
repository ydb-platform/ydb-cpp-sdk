<<<<<<<< HEAD:src/client/topic/proto_accessor.cpp
#include <ydb-cpp-sdk/client/proto/accessor.h>
========
#include <src/client/ydb_proto/accessor.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_topic/proto_accessor.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_topic/proto_accessor.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {
    const Ydb::Topic::DescribeTopicResult& TProtoAccessor::GetProto(const NTopic::TTopicDescription& topicDescription) {
        return topicDescription.GetProto();
    }

    const Ydb::Topic::DescribeConsumerResult& TProtoAccessor::GetProto(const NTopic::TConsumerDescription& consumerDescription) {
        return consumerDescription.GetProto();
    }

    Ydb::Topic::MeteringMode TProtoAccessor::GetProto(NTopic::EMeteringMode mode) {
        switch (mode) {
        case NTopic::EMeteringMode::Unspecified:
            return Ydb::Topic::METERING_MODE_UNSPECIFIED;
        case NTopic::EMeteringMode::RequestUnits:
            return Ydb::Topic::METERING_MODE_REQUEST_UNITS;
        case NTopic::EMeteringMode::ReservedCapacity:
            return Ydb::Topic::METERING_MODE_RESERVED_CAPACITY;
        case NTopic::EMeteringMode::Unknown:
            return Ydb::Topic::METERING_MODE_UNSPECIFIED;
        }
    }

    NTopic::EMeteringMode TProtoAccessor::FromProto(Ydb::Topic::MeteringMode mode) {
        switch (mode) {
        case Ydb::Topic::MeteringMode::METERING_MODE_UNSPECIFIED:
            return NTopic::EMeteringMode::Unspecified;
        case Ydb::Topic::MeteringMode::METERING_MODE_REQUEST_UNITS:
            return NTopic::EMeteringMode::RequestUnits;
        case Ydb::Topic::MeteringMode::METERING_MODE_RESERVED_CAPACITY:
            return NTopic::EMeteringMode::ReservedCapacity;
        default:
            return NTopic::EMeteringMode::Unknown;
        }
    }
}// namespace NYdb

