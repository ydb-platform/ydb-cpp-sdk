#include "key_value.h"
#include "userver_table_client.h"

#include <userver/engine/async.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/engine/future_status.hpp>
#include <userver/engine/semaphore.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/wait_any.hpp>
#include <userver/ydb/exceptions.hpp>
#include <userver/ydb/query.hpp>
#include <userver/ydb/settings.hpp>
#include <userver/ydb/table.hpp>

#include <util/string/printf.h>
#include <util/system/getpid.h>

#include <csignal>
#include <functional>

using namespace NYdb;
using namespace NYdb::NTable;

namespace uydb = userver::ydb;

namespace {

// Shared stop flag for signal handling
std::atomic<bool> ShouldStop{false};

// Show progress callback
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

// Execute a query with optional application timeout and preventive request
void ExecuteWithTimeout(
    uydb::TableClient& client,
    const uydb::OperationSettings& settings,
    const uydb::Query& query,
    const std::function<uydb::PreparedArgsBuilder()>& makeParams,
    TStat& stats,
    std::atomic<std::uint64_t>& succeeded,
    std::atomic<std::uint64_t>& failed,
    bool useApplicationTimeout,
    bool sendPreventiveRequest,
    TDuration reactionTime)
{
    auto stat = stats.StartRequest();

    auto doQuery = [&client, &settings, &query](uydb::PreparedArgsBuilder queryParams) {
        client.ExecuteDataQuery(settings, query, std::move(queryParams));
    };

    try {
        if (!useApplicationTimeout && !sendPreventiveRequest) {
            doQuery(makeParams());

            TSloRequestFinish finish;
            finish.StatusLabel = "SUCCESS";
            stats.FinishRequest(stat, finish);
            succeeded.fetch_add(1);
            return;
        }

        if (useApplicationTimeout && !sendPreventiveRequest) {
            auto task = userver::engine::AsyncNoSpan(
                [&doQuery, &makeParams]() {
                    doQuery(makeParams());
                }
            );

            if (task.WaitNothrowUntil(userver::engine::Deadline::FromDuration(
                    std::chrono::milliseconds(reactionTime.MilliSeconds())))
                == userver::engine::FutureStatus::kReady) {
                task.Get(); // may throw
                TSloRequestFinish finish;
                finish.StatusLabel = "SUCCESS";
                stats.FinishRequest(stat, finish);
                succeeded.fetch_add(1);
            } else {
                task.RequestCancel();
                TSloRequestFinish finish;
                finish.ApplicationTimeout = true;
                stats.FinishRequest(stat, finish);
                failed.fetch_add(1);
            }
            return;
        }

        if (sendPreventiveRequest) {
            auto task1 = userver::engine::AsyncNoSpan(
                [&doQuery, &makeParams]() {
                    doQuery(makeParams());
                }
            );

            userver::engine::SleepFor(
                std::chrono::milliseconds(reactionTime.MilliSeconds() / 2));

            if (task1.IsFinished()) {
                task1.Get(); // may throw
                TSloRequestFinish finish;
                finish.StatusLabel = "SUCCESS";
                stats.FinishRequest(stat, finish);
                succeeded.fetch_add(1);
                return;
            }

            auto task2 = userver::engine::AsyncNoSpan(
                [&doQuery, &makeParams]() {
                    doQuery(makeParams());
                }
            );

            auto idx = userver::engine::WaitAny(task1, task2);

            if (idx.has_value()) {
                try {
                    if (*idx == 0) {
                        task1.Get();
                    } else {
                        task2.Get();
                    }
                    TSloRequestFinish finish;
                    finish.StatusLabel = "SUCCESS";
                    stats.FinishRequest(stat, finish);
                    succeeded.fetch_add(1);
                } catch (const uydb::YdbResponseError& e) {
                    TSloRequestFinish finish;
                    finish.StatusLabel = YdbStatusToString(e.GetStatus().GetStatus());
                    stats.FinishRequest(stat, finish);
                    failed.fetch_add(1);
                }
            } else if (useApplicationTimeout) {
                TSloRequestFinish finish;
                finish.ApplicationTimeout = true;
                stats.FinishRequest(stat, finish);
                failed.fetch_add(1);
            }
            // Cancel remaining tasks
            task1.RequestCancel();
            task2.RequestCancel();
            return;
        }
    } catch (const uydb::YdbResponseError& e) {
        TSloRequestFinish finish;
        finish.StatusLabel = YdbStatusToString(e.GetStatus().GetStatus());
        stats.FinishRequest(stat, finish);
        failed.fetch_add(1);
    } catch (const std::exception& e) {
        TSloRequestFinish finish;
        finish.StatusLabel = "UNKNOWN_ERROR";
        stats.FinishRequest(stat, finish);
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

    // Stats objects
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
            "read",
            MakeNativeSloOtelResourceAttributes(),
            NativeSloMeterSchemaVersion()
        );
    }
    if (!runOptions.DontRunB) {
        writeStats = std::make_unique<TStat>(
            runOptions.CommonOptions.DontPushMetrics ? std::nullopt : std::make_optional(runOptions.CommonOptions.MetricsPushUrl),
            "write",
            MakeNativeSloOtelResourceAttributes(),
            NativeSloMeterSchemaVersion()
        );
    }

    // Set up progress reporter
    TProgressReporter reporter;
    reporter.ReadStats = readStats.get();
    reporter.WriteStats = writeStats.get();
    reporter.ReadSucceeded = &readSucceeded;
    reporter.ReadFailed = &readFailed;
    reporter.WriteSucceeded = &writeSucceeded;
    reporter.WriteFailed = &writeFailed;
    reporter.WriteGenerated = &writeGenerated;
    GlobalReporter = &reporter;

    // Set up signal handlers
    ShouldStop.store(false);
    signal(SIGUSR1, [](int) {
        Cout << TInstant::Now().ToRfc822StringLocal() << " SIGUSR1 handle" << Endl;
        if (GlobalReporter) {
            GlobalReporter->ShowProgress();
        }
    });
    signal(SIGINT, [](int) {
        Cerr << TInstant::Now().ToRfc822StringLocal() << " SIGINT received. Stopping..." << Endl;
        ShouldStop.store(true);
    });

    TInstant start = TInstant::Now();
    TInstant deadline = start + TDuration::Seconds(runOptions.CommonOptions.SecondsToRun);

    Cout << "Jobs launched. Do 'kill -USR1 " << GetPID()
        << "' for progress details or 'kill -INT " << GetPID() << "' (Ctrl/Cmd + C) to interrupt" << Endl
        << "           Start time: " << start.ToRfc822StringLocal() << Endl
        << "Estimated finish time: " << deadline.ToRfc822StringLocal() << Endl;

    std::vector<userver::engine::TaskWithResult<void>> jobTasks;

    // Thread A: Read workload
    if (!runOptions.DontRunA) {
        auto readRps = runOptions.Read_rps;
        auto readTimeout = runOptions.ReadTimeout;
        auto useAppTimeout = runOptions.CommonOptions.UseApplicationTimeout;
        auto sendPreventive = runOptions.CommonOptions.SendPreventiveRequest;
        auto maxInfly = runOptions.CommonOptions.MaxInfly;
        auto objectIdRange = static_cast<std::uint32_t>(maxId * 1.25); // 20% no-result reads

        const TString readQuery = Sprintf(R"(
--!syntax_v1
PRAGMA TablePathPrefix("%s");

DECLARE $object_id_key AS Uint32;
DECLARE $object_id AS Uint32;

SELECT * FROM `%s` WHERE `object_id_key` = $object_id_key AND `object_id` = $object_id;

)", prefix.c_str(), TableName.c_str());

        jobTasks.push_back(userver::engine::AsyncNoSpan(
            [&ydbClient, &readStats, &readSucceeded, &readFailed,
             readRps, readTimeout, useAppTimeout, sendPreventive, maxInfly,
             objectIdRange, readQuery, deadline]() {

                readStats->Start();
                TRpsProvider rpsProvider(readRps);
                rpsProvider.Reset();

                userver::engine::Semaphore semaphore{
                    static_cast<userver::engine::Semaphore::Counter>(maxInfly)};

                std::vector<userver::engine::TaskWithResult<void>> tasks;

                while (!ShouldStop.load() && TInstant::Now() < deadline) {
                    std::uint32_t idToSelect = RandomNumber<std::uint32_t>() % objectIdRange;
                    const std::uint32_t objectIdKey = GetHash(idToSelect);

                    rpsProvider.Use();

                    semaphore.lock_shared();

                    tasks.push_back(userver::engine::AsyncNoSpan(
                        [&ydbClient, &semaphore, &readStats, &readSucceeded, &readFailed,
                         readTimeout, useAppTimeout, sendPreventive,
                         readQuery, objectIdKey, idToSelect]() {

                            uydb::OperationSettings settings;
                            settings.client_timeout_ms = std::chrono::milliseconds(readTimeout.MilliSeconds());

                            const auto makeParams = [&ydbClient, objectIdKey, idToSelect]() {
                                auto builder = ydbClient.GetBuilder();
                                builder.Add("$object_id_key", objectIdKey);
                                builder.Add("$object_id", idToSelect);
                                return builder;
                            };

                            ExecuteWithTimeout(
                                ydbClient, settings, uydb::Query{readQuery},
                                makeParams, *readStats,
                                readSucceeded, readFailed,
                                useAppTimeout, sendPreventive, readTimeout);

                            semaphore.unlock_shared();
                        }
                    ));
                }

                // Wait for all read tasks
                for (auto& task : tasks) {
                    task.Wait();
                }

                readStats->Finish();
            }
        ));
    }

    // Thread B: Write workload
    if (!runOptions.DontRunB) {
        auto writeRps = runOptions.Write_rps;
        auto writeTimeout = runOptions.WriteTimeout;
        auto useAppTimeout = runOptions.CommonOptions.UseApplicationTimeout;
        auto sendPreventive = runOptions.CommonOptions.SendPreventiveRequest;
        auto maxInfly = runOptions.CommonOptions.MaxInfly;
        auto minLength = runOptions.CommonOptions.MinLength;
        auto maxLength = runOptions.CommonOptions.MaxLength;

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
             writeRps, writeTimeout, useAppTimeout, sendPreventive, maxInfly,
             minLength, maxLength, maxId, writeQuery, deadline]() {

                writeStats->Start();
                TRpsProvider rpsProvider(writeRps);
                rpsProvider.Reset();

                TSloGeneratorOptions genOpts;
                genOpts.MinLength = minLength;
                genOpts.MaxLength = maxLength;
                TKeyValueGenerator generator(genOpts, maxId);

                userver::engine::Semaphore semaphore{
                    static_cast<userver::engine::Semaphore::Counter>(maxInfly)};

                std::vector<userver::engine::TaskWithResult<void>> tasks;

                while (!ShouldStop.load() && TInstant::Now() < deadline) {
                    const auto value = BuildValueFromRecord(generator.Get());
                    writeGenerated.fetch_add(1);

                    rpsProvider.Use();

                    semaphore.lock_shared();

                    tasks.push_back(userver::engine::AsyncNoSpan(
                        [&ydbClient, &semaphore, &writeStats, &writeSucceeded, &writeFailed,
                         writeTimeout, useAppTimeout, sendPreventive,
                         writeQuery, value]() {

                            uydb::OperationSettings settings;
                            settings.client_timeout_ms = std::chrono::milliseconds(writeTimeout.MilliSeconds());

                            const auto makeParams = [value]() {
                                return userver_slo::PackValuesToPreparedArgs({value});
                            };

                            ExecuteWithTimeout(
                                ydbClient, settings, uydb::Query{writeQuery},
                                makeParams, *writeStats,
                                writeSucceeded, writeFailed,
                                useAppTimeout, sendPreventive, writeTimeout);

                            semaphore.unlock_shared();
                        }
                    ));
                }

                // Wait for all write tasks
                for (auto& task : tasks) {
                    task.Wait();
                }

                writeStats->Finish();
            }
        ));
    }

    // Wait for all job tasks (read + write loops)
    for (auto& task : jobTasks) {
        task.Wait();
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
