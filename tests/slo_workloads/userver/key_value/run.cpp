#include "key_value.h"
#include "userver_table_client.h"

#include <userver/engine/async.hpp>
#include <userver/engine/semaphore.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/ydb/exceptions.hpp>
#include <userver/ydb/query.hpp>
#include <userver/ydb/settings.hpp>
#include <userver/ydb/table.hpp>

#include <util/string/printf.h>
#include <util/system/getpid.h>

#include <atomic>
#include <csignal>

using namespace NYdb;
using namespace NYdb::NTable;

namespace uydb = userver::ydb;

namespace {

std::atomic<bool> ShouldStop{false};
std::atomic<bool> ShowProgressRequested{false};

class TUserverRpsProvider {
public:
    explicit TUserverRpsProvider(std::uint64_t rps)
        : Rps_(rps)
        , Period_(Max(TDuration::MilliSeconds(10), TDuration::MicroSeconds(1000000 / Rps_)))
    {}

    void Reset() { ProcessedTime_ = TInstant::Now() - Period_ - Period_; }

    void Use() {
        if (Allowed_) {
            --Allowed_;
            return;
        }
        while (!TryUse()) {
            userver::engine::SleepFor(std::chrono::microseconds(Period_.MicroSeconds()));
        }
    }

private:
    bool TryUse() {
        const TInstant now = TInstant::Now();
        Allowed_ = Rps_ * TDuration(now - ProcessedTime_).MicroSeconds() / 1000000;
        if (Allowed_) {
            ProcessedTime_ += TDuration::MicroSeconds(1000000 * Allowed_ / Rps_);
            --Allowed_;
            return true;
        }
        return false;
    }

    std::uint64_t Rps_;
    TDuration Period_;
    TInstant ProcessedTime_;
    std::uint32_t Allowed_ = 0;
};

void DrainCompletedTasks(std::vector<userver::engine::TaskWithResult<void>>& tasks) {
    for (auto& task : tasks) {
        task.Wait();
    }
    tasks.clear();
}

struct TProgressReporter {
    TStat* ReadStats = nullptr;
    TStat* WriteStats = nullptr;
    std::atomic<std::uint64_t>* ReadSucceeded = nullptr;
    std::atomic<std::uint64_t>* ReadFailed = nullptr;
    std::atomic<std::uint64_t>* WriteSucceeded = nullptr;
    std::atomic<std::uint64_t>* WriteFailed = nullptr;
    std::atomic<std::uint64_t>* WriteGenerated = nullptr;

    void ShowProgress() const {
        TStringBuilder report;
        if (ReadStats) {
            report << Endl << "======- ReadJob report (Thread A) -======" << Endl;
            report << "Succeeded: " << ReadSucceeded->load()
                   << ", Failed: " << ReadFailed->load() << Endl;
            ReadStats->PrintStatistics(report);
            report << "========================================" << Endl;
        }
        if (WriteStats) {
            report << Endl << "=====- WriteJob report (Thread B) -=====" << Endl;
            report << "Generated: " << WriteGenerated->load()
                   << ", Succeeded: " << WriteSucceeded->load()
                   << ", Failed: " << WriteFailed->load() << Endl;
            WriteStats->PrintStatistics(report);
            report << "==========================================" << Endl;
        }
        Cout << report;
    }
};

TProgressReporter* GlobalReporter = nullptr;

void PollSignals() {
    if (ShowProgressRequested.exchange(false, std::memory_order_relaxed)) {
        Cout << TInstant::Now().ToRfc822StringLocal() << " SIGUSR1 handle" << Endl;
        if (GlobalReporter) {
            GlobalReporter->ShowProgress();
        }
    }
}

void ExecuteQuery(
    uydb::TableClient& client,
    const uydb::OperationSettings& settings,
    const uydb::Query& query,
    uydb::PreparedArgsBuilder params,
    TStat& stats,
    std::atomic<std::uint64_t>& succeeded,
    std::atomic<std::uint64_t>& failed)
{
    auto stat = stats.StartRequest();
    try {
        client.ExecuteDataQuery(settings, query, std::move(params));
        stats.FinishRequest(stat, TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues()));
        succeeded.fetch_add(1);
    } catch (const uydb::YdbResponseError& e) {
        stats.FinishRequest(stat, TStatus(e.GetStatus().GetStatus(), NYdb::NIssue::TIssues()));
        failed.fetch_add(1);
    } catch (const std::exception&) {
        stats.FinishRequest(stat, TStatus(EStatus::CLIENT_INTERNAL_ERROR, NYdb::NIssue::TIssues()));
        failed.fetch_add(1);
    }
}

} // namespace

int DoRun(TDatabaseOptions& dbOptions, int argc, char** argv) {
    TRunOptions runOptions{ {dbOptions} };
    if (!ParseOptionsRun(argc, argv, runOptions)) {
        return EXIT_FAILURE;
    }

    Cout << TInstant::Now().ToRfc822StringLocal() << " Creating and initializing jobs..." << Endl;

    std::uint32_t maxId = GetTableStats(dbOptions, TableName).MaxId;

    const std::string& prefix = dbOptions.Prefix;

    auto& ydbClient = userver_slo::GetTableClient();

    std::unique_ptr<TStat> readStats;
    std::unique_ptr<TStat> writeStats;

    std::atomic<std::uint64_t> readSucceeded{0};
    std::atomic<std::uint64_t> readFailed{0};
    std::atomic<std::uint64_t> writeSucceeded{0};
    std::atomic<std::uint64_t> writeFailed{0};
    std::atomic<std::uint64_t> writeGenerated{0};

    if (!runOptions.DontRunA) {
        readStats = std::make_unique<TStat>(
            runOptions.CommonOptions.DontPushMetrics ? std::nullopt : std::make_optional(runOptions.CommonOptions.MetricsPushUrl),
            "read"
        );
    }
    if (!runOptions.DontRunB) {
        writeStats = std::make_unique<TStat>(
            runOptions.CommonOptions.DontPushMetrics ? std::nullopt : std::make_optional(runOptions.CommonOptions.MetricsPushUrl),
            "write"
        );
    }

    TProgressReporter reporter;
    reporter.ReadStats = readStats.get();
    reporter.WriteStats = writeStats.get();
    reporter.ReadSucceeded = &readSucceeded;
    reporter.ReadFailed = &readFailed;
    reporter.WriteSucceeded = &writeSucceeded;
    reporter.WriteFailed = &writeFailed;
    reporter.WriteGenerated = &writeGenerated;
    GlobalReporter = &reporter;

    ShouldStop.store(false);
    ShowProgressRequested.store(false);
    signal(SIGUSR1, [](int) { ShowProgressRequested.store(true, std::memory_order_relaxed); });
    signal(SIGINT, [](int) { ShouldStop.store(true, std::memory_order_relaxed); });

    TInstant start = TInstant::Now();
    TInstant deadline = start + TDuration::Seconds(runOptions.CommonOptions.SecondsToRun);

    Cout << "Jobs launched. Do 'kill -USR1 " << GetPID()
        << "' for progress details or 'kill -INT " << GetPID() << "' (Ctrl/Cmd + C) to interrupt" << Endl
        << "           Start time: " << start.ToRfc822StringLocal() << Endl
        << "Estimated finish time: " << deadline.ToRfc822StringLocal() << Endl;

    std::vector<userver::engine::TaskWithResult<void>> jobTasks;

    if (!runOptions.DontRunA) {
        auto readRps = runOptions.Read_rps;
        auto readTimeout = runOptions.ReadTimeout;
        auto maxInfly = runOptions.CommonOptions.MaxInfly;
        auto objectIdRange = static_cast<std::uint32_t>(maxId * 1.25);

        const TString readQuery = Sprintf(R"(
--!syntax_v1
PRAGMA TablePathPrefix("%s");

DECLARE $object_id_key AS Uint32;
DECLARE $object_id AS Uint32;

SELECT * FROM `%s` WHERE `object_id_key` = $object_id_key AND `object_id` = $object_id;

)", prefix.c_str(), TableName.c_str());

        jobTasks.push_back(userver::engine::AsyncNoSpan(
            [&ydbClient, &readStats, &readSucceeded, &readFailed,
             readRps, readTimeout, maxInfly,
             objectIdRange, readQuery, deadline]() {

                readStats->Start();
                TUserverRpsProvider rpsProvider(readRps);
                rpsProvider.Reset();

                userver::engine::Semaphore semaphore{
                    static_cast<userver::engine::Semaphore::Counter>(maxInfly)};

                std::vector<userver::engine::TaskWithResult<void>> tasks;

                while (!ShouldStop.load() && TInstant::Now() < deadline) {
                    PollSignals();

                    std::uint32_t idToSelect = RandomNumber<std::uint32_t>() % objectIdRange;
                    const std::uint32_t objectIdKey = GetHash(idToSelect);

                    rpsProvider.Use();

                    semaphore.lock_shared();

                    tasks.push_back(userver::engine::AsyncNoSpan(
                        [&ydbClient, &semaphore, &readStats, &readSucceeded, &readFailed,
                         readTimeout, readQuery, objectIdKey, idToSelect]() {

                            uydb::OperationSettings settings;
                            settings.client_timeout_ms = std::chrono::milliseconds(readTimeout.MilliSeconds());

                            auto builder = ydbClient.GetBuilder();
                            builder.Add("$object_id_key", objectIdKey);
                            builder.Add("$object_id", idToSelect);

                            ExecuteQuery(
                                ydbClient, settings, uydb::Query{readQuery},
                                std::move(builder), *readStats,
                                readSucceeded, readFailed);

                            semaphore.unlock_shared();
                        }
                    ));

                    if (tasks.size() >= static_cast<size_t>(maxInfly)) {
                        DrainCompletedTasks(tasks);
                    }
                }

                for (auto& task : tasks) {
                    task.Wait();
                }

                readStats->Finish();
            }
        ));
    }

    if (!runOptions.DontRunB) {
        auto writeRps = runOptions.Write_rps;
        auto writeTimeout = runOptions.WriteTimeout;
        auto maxInfly = runOptions.CommonOptions.MaxInfly;

        const TCommonOptions writeCommon = runOptions.CommonOptions;

        const TString writeQuery = Sprintf(R"(
--!syntax_v1
PRAGMA TablePathPrefix("%s");

DECLARE $items AS
    List<Struct<
        `object_id_key`: Uint32,
        `object_id`: Uint32,
        `timestamp`: Uint64,
        `payload`: Utf8>>;

UPSERT INTO `%s` SELECT * FROM AS_TABLE($items);

)", prefix.c_str(), TableName.c_str());

        jobTasks.push_back(userver::engine::AsyncNoSpan(
            [&ydbClient, &writeStats, &writeSucceeded, &writeFailed, &writeGenerated,
             writeRps, writeTimeout, maxInfly,
             writeCommon, maxId, writeQuery, deadline]() {

                writeStats->Start();
                TUserverRpsProvider rpsProvider(writeRps);
                rpsProvider.Reset();

                TKeyValueGenerator generator(writeCommon, maxId);

                userver::engine::Semaphore semaphore{
                    static_cast<userver::engine::Semaphore::Counter>(maxInfly)};

                std::vector<userver::engine::TaskWithResult<void>> tasks;

                while (!ShouldStop.load() && TInstant::Now() < deadline) {
                    PollSignals();

                    const auto value = BuildValueFromRecord(generator.Get());
                    writeGenerated.fetch_add(1);

                    rpsProvider.Use();

                    semaphore.lock_shared();

                    tasks.push_back(userver::engine::AsyncNoSpan(
                        [&ydbClient, &semaphore, &writeStats, &writeSucceeded, &writeFailed,
                         writeTimeout, writeQuery, value]() {

                            uydb::OperationSettings settings;
                            settings.client_timeout_ms = std::chrono::milliseconds(writeTimeout.MilliSeconds());

                            auto params = userver_slo::PackValuesToPreparedArgs({value});

                            ExecuteQuery(
                                ydbClient, settings, uydb::Query{writeQuery},
                                std::move(params), *writeStats,
                                writeSucceeded, writeFailed);

                            semaphore.unlock_shared();
                        }
                    ));

                    if (tasks.size() >= static_cast<size_t>(maxInfly)) {
                        DrainCompletedTasks(tasks);
                    }
                }

                for (auto& task : tasks) {
                    task.Wait();
                }

                writeStats->Finish();
            }
        ));
    }

    for (auto& task : jobTasks) {
        task.Wait();
    }

    if (ShouldStop.load()) {
        Cerr << TInstant::Now().ToRfc822StringLocal() << " SIGINT received. Stopping..." << Endl;
    }

    Cout << "All jobs finished: " << TInstant::Now().ToRfc822StringLocal() << Endl;

    reporter.ShowProgress();

    GlobalReporter = nullptr;

    return EXIT_SUCCESS;
}

int DoCleanup(TDatabaseOptions& dbOptions, int argc) {
    if (argc > 1) {
        Cerr << "Unexpected arguments after cleanup" << Endl;
        return EXIT_FAILURE;
    }
    return DropTable(dbOptions);
}
