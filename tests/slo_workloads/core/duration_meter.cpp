#include "duration_meter.h"

TDurationMeter::TDurationMeter(TDuration& value)
    : Value(value)
    , StartTime(TInstant::Now())
{
}

TDurationMeter::~TDurationMeter() {
    Value += TInstant::Now() - StartTime;
}
