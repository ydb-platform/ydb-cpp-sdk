#pragma once

#include <util/datetime/base.h>

struct TDurationMeter {
    TDurationMeter(TDuration& value);
    ~TDurationMeter();

    TDuration& Value;
    TInstant StartTime;
};
