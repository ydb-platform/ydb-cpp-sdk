#pragma once

#include <util/datetime/base.h>

inline constexpr TDuration DefaultReactionTime = TDuration::Minutes(2);
inline constexpr TDuration ReactionTimeDelay = TDuration::MilliSeconds(5);
inline constexpr std::uint64_t PartitionsCount = 64;
