#include "key_value.h"
#include "userver_table_client.h"

#include <userver/engine/async.hpp>
#include <userver/engine/semaphore.hpp>
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

} // namespace

int DoCreate(TDatabaseOptions& dbOptions, int argc, char** argv) {
    TCreateOptions createOptions{ {dbOptions} };
    if (!ParseOptionsCreate(argc, argv, createOptions)) {
        return EXIT_FAILURE;
    }

    createOptions.CommonOptions.MaxInfly = createOptions.CommonOptions.MaxInputThreads;

    int result = CreateTable(dbOptions);
    if (result) {
        return result;
    }

    std::uint32_t maxId = GetTableStats(dbOptions, TableName).MaxId;

    createOptions.CommonOptions.ReactionTime = TDuration::Seconds(20);

    Cout << TInstant::Now().ToRfc822StringLocal() << " Uploading initial content... do 'kill -USR1 " << GetPID()
        << "' for progress details or Ctrl/Cmd+C to interrupt" << Endl;

    ShouldStop.store(false);
    signal(SIGINT, [](int) { ShouldStop.store(true, std::memory_order_relaxed); });

    const auto& opts = createOptions.CommonOptions;
    const std::string& prefix = opts.DatabaseOptions.Prefix;

    auto& ydbClient = userver_slo::GetTableClient();

    userver::engine::AsyncNoSpan([&]() {
        TStat stats(
            opts.DontPushMetrics ? std::nullopt : std::make_optional(opts.MetricsPushUrl),
            "generate"
        );
        stats.Start();

        TPackGenerator<TKeyValueGenerator, TKeyValueRecordData> packGenerator(
            opts,
            createOptions.PackSize,
            &BuildValueFromRecord,
            createOptions.Count,
            maxId
        );

        userver::engine::Semaphore semaphore{
            static_cast<userver::engine::Semaphore::Counter>(opts.MaxInfly)};

        std::atomic<std::uint64_t> succeeded{0};
        std::atomic<std::uint64_t> failed{0};
        std::atomic<std::uint64_t> total{0};

        const TString query = Sprintf(R"(
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

        std::vector<userver::engine::TaskWithResult<void>> tasks;

        std::vector<TValue> pack;
        while (!ShouldStop.load() && packGenerator.GetNextPack(pack)) {
            semaphore.lock_shared();
            total.fetch_add(1);

            auto params = userver_slo::PackValuesToPreparedArgs(pack);

            tasks.push_back(userver::engine::AsyncNoSpan(
                [&ydbClient, &semaphore, &stats, &succeeded, &failed,
                 &opts, query, params = std::move(params)]() mutable {
                    auto stat = stats.StartRequest();
                    try {
                        uydb::OperationSettings settings;
                        settings.client_timeout_ms = std::chrono::milliseconds(opts.ReactionTime.MilliSeconds());

                        ydbClient.ExecuteDataQuery(settings, uydb::Query{query}, std::move(params));

                        stats.FinishRequest(stat, TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues()));
                        succeeded.fetch_add(1);
                    } catch (const uydb::YdbResponseError& e) {
                        stats.FinishRequest(stat, TStatus(e.GetStatus().GetStatus(), NYdb::NIssue::TIssues()));
                        failed.fetch_add(1);
                    } catch (const std::exception&) {
                        stats.FinishRequest(stat, TStatus(EStatus::CLIENT_INTERNAL_ERROR, NYdb::NIssue::TIssues()));
                        failed.fetch_add(1);
                    }
                    semaphore.unlock_shared();
                }
            ));
        }

        for (auto& task : tasks) {
            task.Wait();
        }

        stats.Finish();

        TStringBuilder report;
        report << Endl << "======- GenerateInitialContentJob report -======" << Endl;
        report << "Total: " << total.load() << ", Succeeded: " << succeeded.load()
               << ", Failed: " << failed.load() << Endl;
        stats.PrintStatistics(report);
        report << "========================================" << Endl;
        Cout << report;
    }).Wait();

    return EXIT_SUCCESS;
}
