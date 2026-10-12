#include <tests/slo_workloads/userver/key_value/key_value.h>
#include <tests/slo_workloads/userver/key_value/userver_table_client.h>

#include <userver/engine/run_standalone.hpp>
#include <userver/ydb/exceptions.hpp>
#include <userver/ydb/response.hpp>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {
std::string Mode;
std::atomic<unsigned> Calls{0};
}

userver::ydb::ExecuteResponse WrappedQuery(
    userver::ydb::TableClient*,
    userver::ydb::OperationSettings,
    const userver::ydb::Query&,
    userver::ydb::PreparedArgsBuilder&&) asm(QUERY_WRAP_SYMBOL);

userver::ydb::ExecuteResponse WrappedQuery(
    userver::ydb::TableClient*,
    userver::ydb::OperationSettings,
    const userver::ydb::Query&,
    userver::ydb::PreparedArgsBuilder&&)
{
    auto call = Calls.fetch_add(1);
    bool failed = Mode != "healthy" && Mode != "recovered" && Mode != "planned";
    if (Mode == "planned" && call == 0) {
        std::raise(SIGINT);
    }
    if (Mode == "error-then-success" && call > 0) {
        failed = false;
    }
    if (failed) {
        auto cause = "CPP_USERVER_final_" + Mode;
        std::cerr << cause << std::endl;
        if (Mode == "deadline") {
            throw userver::ydb::DeadlineExceededError(cause);
        }
        NYdb::NIssue::TIssues issues;
        issues.AddIssue(NYdb::NIssue::TIssue(cause));
        throw userver::ydb::YdbResponseError(
            cause,
            NYdb::TStatus(NYdb::EStatus::BAD_REQUEST, std::move(issues)));
    }
    return userver::ydb::ExecuteResponse(
        NYdb::NQuery::TExecuteQueryResult(
            NYdb::TStatus(NYdb::EStatus::SUCCESS, NYdb::NIssue::TIssues())));
}

TTableStats WrappedStats(
    TDatabaseOptions&, const std::string&) asm(STATS_WRAP_SYMBOL);

TTableStats WrappedStats(TDatabaseOptions&, const std::string&) {
    return TTableStats{1, 1};
}

int WrappedCreate(TDatabaseOptions&) asm(CREATE_WRAP_SYMBOL);

int WrappedCreate(TDatabaseOptions&) {
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return 2;
    }
    Mode = argv[1];
    int result = 2;
    userver::engine::RunStandalone(2, [&] {
        try {
        NYdb::TDriver driver(
            NYdb::TDriverConfig().SetEndpoint("127.0.0.1:9").SetDatabase("/probe"));
        std::string prefix = "/probe";
        TDatabaseOptions options{driver, prefix};
        userver_slo::InitTableClient("127.0.0.1:9", "/probe", {}, "", false);
        std::vector<std::string> args = Mode == "preload"
            ? std::vector<std::string>{"create", "--count", "1", "--pack-size", "1", "--dont-push"}
            : std::vector<std::string>{"run", "--time", "1", "--read-rps", "10",
                                      "--write-rps", "10", "--infly", "1", "--dont-push"};
        if (Mode == "read" || Mode == "deadline") {
            args.push_back("--no-write");
        } else if (Mode == "write") {
            args.push_back("--no-read");
        }
        std::vector<char*> pointers;
        for (auto& arg : args) {
            pointers.push_back(arg.data());
        }
        result = Mode == "preload"
            ? DoCreate(options, pointers.size(), pointers.data())
            : DoRun(options, pointers.size(), pointers.data());
        userver_slo::ShutdownTableClient();
        driver.Stop(true);
        } catch (const std::exception& error) {
            std::cerr << "CPP_USERVER_SETUP_ERROR " << error.what() << std::endl;
            userver_slo::ShutdownTableClient();
            result = 2;
        }
    });
    std::cout << "CPP_USERVER mode=" << Mode << " calls=" << Calls.load()
              << " exit=" << result << std::endl;
    return Calls.load() == 0 ? 2 : result;
}
