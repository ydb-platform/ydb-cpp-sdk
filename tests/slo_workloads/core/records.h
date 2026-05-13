#pragma once

#include <cstdint>
#include <string>

struct TRecordData {
    std::uint32_t ObjectId;
    std::uint64_t Timestamp;
    std::string Guid;
    std::string Payload;
};

struct TKeyValueRecordData {
    std::uint32_t ObjectId;
    std::uint64_t Timestamp;
    std::string Payload;
};

struct TSloGeneratorOptions {
    std::uint32_t MinLength = 20;
    std::uint32_t MaxLength = 200;
};
