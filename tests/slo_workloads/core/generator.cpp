#include "generator.h"

TValueGenerator::TValueGenerator(const TSloGeneratorOptions& opts, ui32 startId)
    : Opts(opts)
    , CurrentObjectId(startId)
{
}

TRecordData TValueGenerator::Get() {
    TDurationMeter computeTime(ComputeTime);
    if (RandomNumber<ui32>() % 10 > 2) {
        ++CurrentObjectId;
    }
    return {
        CurrentObjectId,
        TInstant::Now().MicroSeconds(),
        CreateGuidAsString(),
        SloGenerateRandomString(Opts.MinLength, Opts.MaxLength)
    };
}

TDuration TValueGenerator::GetComputeTime() const {
    return ComputeTime;
}

TKeyValueGenerator::TKeyValueGenerator(const TSloGeneratorOptions& opts, ui32 startId)
    : Opts(opts)
    , CurrentObjectId(startId)
{
}

TKeyValueRecordData TKeyValueGenerator::Get() {
    TDurationMeter computeTime(ComputeTime);
    ++CurrentObjectId;
    return {
        CurrentObjectId,
        TInstant::Now().MicroSeconds(),
        SloGenerateRandomString(Opts.MinLength, Opts.MaxLength)
    };
}

TDuration TKeyValueGenerator::GetComputeTime() const {
    return ComputeTime;
}
