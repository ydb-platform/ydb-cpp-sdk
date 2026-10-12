#include <tests/slo_workloads/key_value/key_value.h>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
std::string Mode;
std::atomic<unsigned> Calls{0};
}

extern "C" NYdb::TAsyncStatus WrappedOperation(
    NYdb::NTable::TTableClient*,
    NYdb::NTable::TTableClient::TOperationFunc&&,
    const NYdb::NTable::TRetryOperationSettings&) asm(OPERATION_WRAP_SYMBOL);

extern "C" NYdb::TAsyncStatus WrappedOperation(
    NYdb::NTable::TTableClient*,
    NYdb::NTable::TTableClient::TOperationFunc&&,
    const NYdb::NTable::TRetryOperationSettings&)
{
    auto call = Calls.fetch_add(1);
    bool failed = Mode != "healthy" && Mode != "recovered" && Mode != "planned";
    if (Mode == "planned" && call == 0) {
        std::raise(SIGINT);
    }
    if (Mode == "error-then-success" && call > 0) {
        failed = false;
    }
    auto status = failed
        ? (Mode == "deadline" ? NYdb::EStatus::TIMEOUT : NYdb::EStatus::BAD_REQUEST)
        : NYdb::EStatus::SUCCESS;
    NYdb::NIssue::TIssues issues;
    if (failed) {
        issues.AddIssue(NYdb::NIssue::TIssue("CPP_PROBE_final_" + Mode));
    }
    return NThreading::MakeFuture(NYdb::TStatus(status, issues));
}

extern "C" TTableStats WrappedStats(
    TDatabaseOptions&, const std::string&) asm(STATS_WRAP_SYMBOL);

extern "C" TTableStats WrappedStats(TDatabaseOptions&, const std::string&) {
    return TTableStats{1, 1};
}

extern "C" int WrappedCreate(TDatabaseOptions&) asm(CREATE_WRAP_SYMBOL);

extern "C" int WrappedCreate(TDatabaseOptions&) {
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return 2;
    }
    Mode = argv[1];
    NYdb::TDriver driver(
        NYdb::TDriverConfig().SetEndpoint("127.0.0.1:9").SetDatabase("/probe"));
    std::string prefix = "/probe";
    TDatabaseOptions options{driver, prefix};
    SetUpInteraction();
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
    int result = Mode == "preload"
        ? DoCreate(options, pointers.size(), pointers.data())
        : DoRun(options, pointers.size(), pointers.data());
    driver.Stop(true);
    std::cout << "CPP_NATIVE mode=" << Mode << " calls=" << Calls.load()
              << " exit=" << result << std::endl;
    if (Calls.load() == 0) {
        return 2;
    }
    return result;
}
