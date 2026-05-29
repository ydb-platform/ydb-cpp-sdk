#include "utils.h"

#include <ydb-cpp-sdk/client/resources/ydb_resources.h>

#include <library/cpp/threading/future/async.h>

#include <util/stream/file.h>

using namespace NLastGetopt;
using namespace NYdb;

#ifdef REF
static constexpr const char* RefLabel = Y_STRINGIZE(REF);
#else
static constexpr const char* RefLabel = "unknown";
#endif

Y_DECLARE_OUT_SPEC(, NYdb::TStatus, stream, value) {
    stream << "Status: " << value.GetStatus() << Endl;
    value.GetIssues().PrintTo(stream);
}

std::map<std::string, std::string> MakeNativeSloOtelResourceAttributes() {
    return {
        {"ref", RefLabel},
        {"sdk", "cpp"},
        {"sdk_version", NYdb::GetSdkSemver()},
    };
}

std::string NativeSloMeterSchemaVersion() {
    return NYdb::GetSdkSemver();
}

using namespace NYdb::NTable;

TParams PackValuesToParamsAsList(const std::vector<TValue>& items, const std::string name) {
    TValueBuilder itemsAsList;
    itemsAsList.BeginList();
    for (const TValue& item : items) {
        itemsAsList.AddListItem(item);
    }
    itemsAsList.EndList();

    TParamsBuilder paramsBuilder;
    paramsBuilder.AddParam(name, itemsAsList.Build());
    return paramsBuilder.Build();
}

std::string YdbStatusToString(NYdb::EStatus status) {
    return SloYdbStatusToString(status);
}

TTableStats GetTableStats(TDatabaseOptions& dbOptions, const std::string& tableName) {
    Cout << TInstant::Now().ToRfc822StringLocal()
         << " Getting table stats (maxId and count of rows) with ReadTable... " << Endl;
    TInstant start_time = TInstant::Now();
    NYdb::NTable::TTableClient client(
        dbOptions.Driver,
        NYdb::NTable::TClientSettings()
            .MinSessionCV(8)
            .AllowRequestMigration(true)
    );

    std::optional<TTablePartIterator> tableIterator;
    NYdb::NStatusHelpers::ThrowOnError(client.RetryOperationSync([&tableIterator, &dbOptions, &tableName](TSession session) {
        auto result = session.ReadTable(
                             JoinPath(dbOptions.Prefix, tableName),
                             TReadTableSettings().AppendColumns("object_id"))
                          .GetValueSync();

        if (result.IsSuccess()) {
            tableIterator = result;
        }

        return result;
    }));
    Y_ENSURE(tableIterator);
    TSimpleThreadPool pool;
    std::vector<NThreading::TFuture<TTableStats>> futures;
    pool.Start(10);
    for (;;) {
        auto tablePart = tableIterator->ReadNext().GetValueSync();
        if (!tablePart.IsSuccess()) {
            if (tablePart.EOS()) {
                break;
            }

            NYdb::NStatusHelpers::ThrowOnError(tablePart);
        }
        futures.push_back(
            NThreading::Async(
                [extractedPart = tablePart.ExtractPart()] {
                    auto rsParser = TResultSetParser(extractedPart);
                    std::uint32_t partMax = 0;
                    while (rsParser.TryNextRow()) {
                        auto& idParser = rsParser.ColumnParser("object_id");
                        idParser.OpenOptional();
                        std::uint32_t id = idParser.GetUint32();
                        if (id > partMax) {
                            partMax = id;
                        }
                    }
                    return TTableStats{rsParser.RowsCount(), partMax};
                },
                pool
            )
        );
    }
    TTableStats result;
    for (auto future : futures) {
        TTableStats partStats = future.GetValueSync();
        if (partStats.MaxId > result.MaxId) {
            result.MaxId = partStats.MaxId;
        }
        result.RowCount += partStats.RowCount;
    }
    Cout << TInstant::Now().ToRfc822StringLocal() << " Done. maxId=" << result.MaxId << ", row count=" << result.RowCount
         << ". Calculations took " << TInstant::Now() - start_time << Endl;
    return result;
}

void ParseOptionsCommon(TOpts& opts, TCommonOptions& options) {
    opts.AddLongOption("threads", "Number of threads to use").RequiredArgument("NUM")
        .DefaultValue(options.MaxInputThreads).StoreResult(&options.MaxInputThreads);
    opts.AddLongOption("stop-on-error", "Stop thread if an error occured").NoArgument()
        .SetFlag(&options.StopOnError).DefaultValue(options.StopOnError);
    opts.AddLongOption("payload-min", "Minimum length of payload string").RequiredArgument("NUM")
        .DefaultValue(options.MinLength).StoreResult(&options.MinLength);
    opts.AddLongOption("payload-max", "Maximum length of payload string").RequiredArgument("NUM")
        .DefaultValue(options.MaxLength).StoreResult(&options.MaxLength);
    opts.AddLongOption("dont-push", "Do not push metrics").NoArgument()
        .SetFlag(&options.DontPushMetrics).DefaultValue(options.DontPushMetrics);
    opts.AddLongOption("metrics-push-url", "URL to push metrics").RequiredArgument("URL")
        .DefaultValue(options.MetricsPushUrl).StoreResult(&options.MetricsPushUrl);
    opts.AddLongOption("app-timeout", "Use application timeout (over SDK)").NoArgument()
        .SetFlag(&options.UseApplicationTimeout).DefaultValue(options.UseApplicationTimeout);
    opts.AddLongOption("prevention-request", "Send prevention request at 1/2 of timeout").NoArgument()
        .SetFlag(&options.SendPreventiveRequest).DefaultValue(options.SendPreventiveRequest);

    opts.MutuallyExclusive("dont-push", "metrics-push-url");
}

bool CheckOptionsCommon(TCommonOptions& options) {
    if (options.MinLength > options.MaxLength) {
        Cerr << "--payload-min should be less than --payload-max" << Endl;
        return false;
    }
    if (!options.MaxInputThreads) {
        Cerr << "--threads should be more than 0" << Endl;
        return false;
    }

    return true;
}

bool ParseOptionsCreate(int argc, char** argv, TCreateOptions& createOptions) {
    TOpts opts = TOpts::Default();
    ParseOptionsCommon(opts, createOptions.CommonOptions);
    opts.AddLongOption("count", "Total number of records to generate").RequiredArgument("NUM")
        .DefaultValue(createOptions.Count).StoreResult(&createOptions.Count);
    opts.AddLongOption("pack-size", "Number of new records in each create request").RequiredArgument("NUM")
        .DefaultValue(createOptions.PackSize).StoreResult(&createOptions.PackSize);

    TOptsParseResult res(&opts, argc, argv);

    if (!CheckOptionsCommon(createOptions.CommonOptions)) {
        return false;
    }
    if (!createOptions.Count) {
        Cerr << "--count should be more than 0" << Endl;
        return false;
    }
    if (!createOptions.PackSize) {
        Cerr << "--pack-size should be more than 0" << Endl;
        return false;
    }
    return true;
}

std::string GetCmdList() {
    return "create, run, cleanup";
}

ECommandType ParseCommand(const char* cmd) {
    if (!strcmp(cmd, "create")) {
        return ECommandType::Create;
    }
    if (!strcmp(cmd, "run")) {
        return ECommandType::Run;
    }
    if (!strcmp(cmd, "cleanup")) {
        return ECommandType::Cleanup;
    }
    return ECommandType::Unknown;
}

bool ParseOptionsRun(int argc, char** argv, TRunOptions& runOptions) {
    TOpts opts = TOpts::Default();
    ParseOptionsCommon(opts, runOptions.CommonOptions);
    opts.AddLongOption("time", "Time to run (Seconds)").RequiredArgument("Seconds")
        .DefaultValue(runOptions.CommonOptions.SecondsToRun).StoreResult(&runOptions.CommonOptions.SecondsToRun);
    opts.AddLongOption("read-rps", "Request generation rate for read requests (Thread A)").RequiredArgument("NUM")
        .DefaultValue(runOptions.Read_rps).StoreResult(&runOptions.Read_rps);
    opts.AddLongOption("write-rps", "Request generation rate for write requests (Thread B)").RequiredArgument("NUM")
        .DefaultValue(runOptions.Write_rps).StoreResult(&runOptions.Write_rps);
    opts.AddLongOption("no-read", "Do not run reading requests (thread A)").NoArgument()
        .SetFlag(&runOptions.DontRunA).DefaultValue(runOptions.DontRunA);
    opts.AddLongOption("no-write", "Do not run writing requests (thread B)").NoArgument()
        .SetFlag(&runOptions.DontRunB).DefaultValue(runOptions.DontRunB);
    opts.AddLongOption("infly", "Maximum number of running jobs").RequiredArgument("NUM")
        .DefaultValue(runOptions.CommonOptions.MaxInfly).StoreResult(&runOptions.CommonOptions.MaxInfly);
    opts.AddLongOption("read-timeout", "Read requests execution timeout [ms]").RequiredArgument("NUM")
        .DefaultValue(runOptions.ReadTimeout).StoreResult(&runOptions.ReadTimeout);
    opts.AddLongOption("write-timeout", "Write requests execution timeout [ms]").RequiredArgument("NUM")
        .DefaultValue(runOptions.WriteTimeout).StoreResult(&runOptions.WriteTimeout);
    TOptsParseResult res(&opts, argc, argv);

    if (!CheckOptionsCommon(runOptions.CommonOptions)) {
        return false;
    }
    if (!runOptions.CommonOptions.SecondsToRun) {
        Cerr << "Time to run should be more than 0" << Endl;
        return false;
    }
    return true;
}
