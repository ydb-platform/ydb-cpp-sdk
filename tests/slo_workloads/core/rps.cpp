#include "rps.h"

#include <util/system/datetime.h>

TRpsProvider::TRpsProvider(std::uint64_t rps)
    : Rps(rps)
    , Period(Max(TDuration::MilliSeconds(10), TDuration::MicroSeconds(1000000 / Rps)))
    , ProcessedTime(TInstant::Now())
{
}

void TRpsProvider::Reset() {
    ProcessedTime = TInstant::Now() - Period - Period;
}

void TRpsProvider::Use() {
    if (Allowed) {
        --Allowed;
        return;
    }

    while (!TryUse()) {
        SleepUntil(TInstant::Now() + Period);
    }
}

bool TRpsProvider::TryUse() {
    TInstant now = TInstant::Now();
    Allowed = Rps * TDuration(now - ProcessedTime).MicroSeconds() / 1000000;
    if (Allowed) {
        ProcessedTime += TDuration::MicroSeconds(1000000 * Allowed / Rps);
        --Allowed;
        return true;
    }
    return false;
}

std::uint64_t TRpsProvider::GetRps() const {
    return Rps;
}
