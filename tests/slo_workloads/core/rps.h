#pragma once

#include <util/datetime/base.h>

#include <cstdint>

class TRpsProvider {
public:
    explicit TRpsProvider(std::uint64_t rps);
    void Reset();
    void Use();
    bool TryUse();
    std::uint64_t GetRps() const;

private:
    std::uint64_t Rps;
    TDuration Period;
    TInstant ProcessedTime;
    TInstant LastCheck;
    std::uint32_t Allowed = 0;
    TInstant StartTime;
};
