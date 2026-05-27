#include "utils.h"

#include <ydb-cpp-sdk/client/resources/ydb_resources.h>

#include <library/cpp/threading/future/async.h>

#include <util/folder/path.h>
#include <util/stream/file.h>
#include <util/string/builder.h>
#include <util/string/cast.h>
#include <util/string/strip.h>
#include <util/system/env.h>

#include <vector>

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

std::string GetCmdList() {
    return "create, run, cleanup (omit to run create -> run -> cleanup in one process)";
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
    switch (status) {
        case NYdb::EStatus::SUCCESS:
            return "SUCCESS";
        case NYdb::EStatus::BAD_REQUEST:
            return "BAD_REQUEST";
        case NYdb::EStatus::UNAUTHORIZED:
            return "UNAUTHORIZED";
        case NYdb::EStatus::INTERNAL_ERROR:
            return "INTERNAL_ERROR";
        case NYdb::EStatus::ABORTED:
            return "ABORTED";
        case NYdb::EStatus::UNAVAILABLE:
            return "UNAVAILABLE";
        case NYdb::EStatus::OVERLOADED:
            return "OVERLOADED";
        case NYdb::EStatus::SCHEME_ERROR:
            return "SCHEME_ERROR";
        case NYdb::EStatus::GENERIC_ERROR:
            return "GENERIC_ERROR";
        case NYdb::EStatus::TIMEOUT:
            return "TIMEOUT";
        case NYdb::EStatus::BAD_SESSION:
            return "BAD_SESSION";
        case NYdb::EStatus::PRECONDITION_FAILED:
            return "PRECONDITION_FAILED";
        case NYdb::EStatus::ALREADY_EXISTS:
            return "ALREADY_EXISTS";
        case NYdb::EStatus::NOT_FOUND:
            return "NOT_FOUND";
        case NYdb::EStatus::SESSION_EXPIRED:
            return "SESSION_EXPIRED";
        case NYdb::EStatus::CANCELLED:
            return "CANCELLED";
        case NYdb::EStatus::UNDETERMINED:
            return "UNDETERMINED";
        case NYdb::EStatus::UNSUPPORTED:
            return "UNSUPPORTED";
        case NYdb::EStatus::SESSION_BUSY:
            return "SESSION_BUSY";
        case NYdb::EStatus::EXTERNAL_ERROR:
            return "EXTERNAL_ERROR";
        case NYdb::EStatus::STATUS_UNDEFINED:
            return "STATUS_UNDEFINED";
        case NYdb::EStatus::TRANSPORT_UNAVAILABLE:
            return "TRANSPORT_UNAVAILABLE";
        case NYdb::EStatus::CLIENT_RESOURCE_EXHAUSTED:
            return "CLIENT_RESOURCE_EXHAUSTED";
        case NYdb::EStatus::CLIENT_DEADLINE_EXCEEDED:
            return "CLIENT_DEADLINE_EXCEEDED";
        case NYdb::EStatus::CLIENT_INTERNAL_ERROR:
            return "CLIENT_INTERNAL_ERROR";
        case NYdb::EStatus::CLIENT_CANCELLED:
            return "CLIENT_CANCELLED";
        case NYdb::EStatus::CLIENT_UNAUTHENTICATED:
            return "CLIENT_UNAUTHENTICATED";
        case NYdb::EStatus::CLIENT_CALL_UNIMPLEMENTED:
            return "CLIENT_CALL_UNIMPLEMENTED";
        case NYdb::EStatus::CLIENT_OUT_OF_RANGE:
            return "CLIENT_OUT_OF_RANGE";
        case NYdb::EStatus::CLIENT_DISCOVERY_FAILED:
            return "CLIENT_DISCOVERY_FAILED";
        case NYdb::EStatus::CLIENT_LIMITS_REACHED:
            return "CLIENT_LIMITS_REACHED";
    }
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
    std::string metricsPushUrlFromEnv = GetEnv("OTEL_EXPORTER_OTLP_METRICS_ENDPOINT");
    if (!metricsPushUrlFromEnv.empty()) {
        options.MetricsPushUrl = metricsPushUrlFromEnv;
    }

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

bool ParseOptionsRun(int argc, char** argv, TRunOptions& runOptions) {
    TOpts opts = TOpts::Default();
    ParseOptionsCommon(opts, runOptions.CommonOptions);

    if (std::string workloadDuration = GetEnv("WORKLOAD_DURATION"); !workloadDuration.empty()) {
        try {
            std::uint32_t parsed = FromString<std::uint32_t>(workloadDuration);
            if (parsed > 0) {
                runOptions.CommonOptions.SecondsToRun = parsed;
            }
        } catch (const std::exception& e) {
            Cerr << "Invalid WORKLOAD_DURATION env value '" << workloadDuration << "': " << e.what() << Endl;
        }
    }

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
