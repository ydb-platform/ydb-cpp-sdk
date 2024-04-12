#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/retry/retry.h
#include <ydb-cpp-sdk/client/types/fluent_settings_helpers.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/client/ydb_types/fluent_settings_helpers.h>
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_retry/retry.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_retry/retry.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb::NRetry {

struct TBackoffSettings {
    using TSelf = TBackoffSettings;

    FLUENT_SETTING_DEFAULT(TDuration, SlotDuration, TDuration::Seconds(1));
    FLUENT_SETTING_DEFAULT(ui32, Ceiling, 6);
    FLUENT_SETTING_DEFAULT(double, UncertainRatio, 0.5);
};

struct TRetryOperationSettings {
    using TSelf = TRetryOperationSettings;

    FLUENT_SETTING_DEFAULT(ui32, MaxRetries, 10);
    FLUENT_SETTING_DEFAULT(bool, RetryNotFound, true);
    FLUENT_SETTING_DEFAULT(TDuration, GetSessionClientTimeout, TDuration::Seconds(5));
    FLUENT_SETTING_DEFAULT(TDuration, MaxTimeout, TDuration::Max());
    FLUENT_SETTING_DEFAULT(TBackoffSettings, FastBackoffSettings, DefaultFastBackoffSettings());
    FLUENT_SETTING_DEFAULT(TBackoffSettings, SlowBackoffSettings, DefaultSlowBackoffSettings());
    FLUENT_SETTING_FLAG(Idempotent);
    FLUENT_SETTING_FLAG(Verbose);

    static TBackoffSettings DefaultFastBackoffSettings() {
        return TBackoffSettings()
            .Ceiling(10)
            .SlotDuration(TDuration::MilliSeconds(5))
            .UncertainRatio(0.5);
    }

    static TBackoffSettings DefaultSlowBackoffSettings() {
        return TBackoffSettings()
            .Ceiling(6)
            .SlotDuration(TDuration::Seconds(1))
            .UncertainRatio(0.5);
    }
};

} // namespace NYdb::NRetry
