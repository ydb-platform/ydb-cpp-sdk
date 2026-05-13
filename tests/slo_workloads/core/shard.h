#pragma once

#include <cstdint>

std::uint32_t SloGetSpecialId(std::uint32_t id);
std::uint32_t SloGetShardSpecialId(std::uint64_t shardNo);
std::uint32_t SloGetHash(std::uint32_t value);
