#include "key_value.h"

using namespace NYdb;

const std::string TableName = "key_value";

NYdb::TValue BuildValueFromRecord(const TKeyValueRecordData& recordData) {
    NYdb::TValueBuilder value;
    value.BeginStruct();
    value.AddMember("object_id_key").Uint32(GetHash(recordData.ObjectId));
    value.AddMember("object_id").Uint32(recordData.ObjectId);
    value.AddMember("timestamp").Uint64(recordData.Timestamp);
    value.AddMember("payload").Utf8(recordData.Payload);
    value.EndStruct();
    return value.Build();
}
