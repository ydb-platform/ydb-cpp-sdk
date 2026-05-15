#include "shard.h"

#include "constants.h"

#include <util/generic/algorithm.h>
#include <util/generic/hash.h>

static double SloShardSizeDouble() {
    return (static_cast<double>(Max<std::uint32_t>()) + 1) / static_cast<double>(PartitionsCount);
}

std::uint32_t SloGetSpecialId(std::uint32_t id) {
    const double shardSize = SloShardSizeDouble();
    return static_cast<std::uint32_t>(id / shardSize) * static_cast<std::uint32_t>(shardSize) + 1;
}

std::uint32_t SloGetShardSpecialId(std::uint64_t shardNo) {
    return static_cast<std::uint32_t>(shardNo * SloShardSizeDouble() + 1);
}

std::uint32_t SloGetHash(std::uint32_t value) {
    std::uint32_t result = NumericHash(value);
    if (result == SloGetSpecialId(result)) {
        ++result;
    }
    return result;
}
