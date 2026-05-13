#pragma once

#include "metrics.h"

#include <util/datetime/base.h>
#include <util/string/builder.h>
#include <util/string/printf.h>
#include <util/thread/pool.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

inline std::string GetMillisecondsStr(const TDuration& d) {
    return TStringBuilder() << d.MilliSeconds() << '.' << Sprintf("%03" PRIu64, d.MicroSeconds() % 1000);
}

inline double GetMillisecondsDouble(const TDuration& d) {
    return static_cast<double>(d.MicroSeconds()) / 1000;
}

struct TSloRequestFinish {
    bool ApplicationTimeout = false;
    std::optional<std::string> StatusLabel;
};

struct TStatUnit {
    TStatUnit(TInstant start)
        : Start(start)
        , RetryAttempts(0)
        , IsFirstAttempt(true)
    {}

    void IncRetryAttempts() {
        if (IsFirstAttempt) {
            IsFirstAttempt = false;
        } else {
            ++RetryAttempts;
        }
    }

    TInstant Start;
    TInstant End;

    std::uint64_t RetryAttempts;
    bool IsFirstAttempt;
};

class TStat {
public:
    TStat(
        const std::optional<std::string>& metricsPushUrl,
        const std::string& operationType,
        const std::map<std::string, std::string>& resourceAttributes,
        const std::string& meterSchemaVersion
    );

    void Start();
    void Finish();

    std::shared_ptr<TStatUnit> StartRequest();
    void FinishRequest(const std::shared_ptr<TStatUnit>& unit, const TSloRequestFinish& finish);

    void ReportMaxInfly();
    void ReportStats(std::uint64_t sessions, std::uint64_t readPromises, std::uint64_t executorPromises);

    void PrintStatistics(TStringBuilder& out);

    TInstant GetStartTime() const;

private:
    void ScheduleMetricsPush(std::function<void()> func);

    std::recursive_mutex Mutex;
    TThreadPool MetricsPushQueue;

    TInstant StartTime;
    TInstant FinishTime;

    std::uint64_t Infly = 0;
    std::uint64_t ActiveSessions = 0;

    std::map<std::string, std::uint64_t> Statuses;
    std::uint64_t CountMaxInfly = 0;
    std::uint64_t ApplicationTimeout = 0;
    std::vector<TDuration> OkDelays;

    std::uint64_t ReadPromises = 0;
    std::uint64_t ExecutorPromises = 0;

    std::unique_ptr<IMetricsPusher> MetricsPusher;
};
