// Userver-specific DoMain() override.
// Wraps command execution inside userver::engine::RunStandalone() so that
// coroutine-based ydb::TableClient and engine primitives work correctly.
// All other utils (ParseOptions*, GetTableStats, YdbStatusToString, etc.)
// come from slo-utils-base via the shared header.

#include "userver_utils.h"
#include "userver_table_client.h"

#include <ydb-cpp-sdk/client/iam/iam.h>

#include <util/folder/path.h>
#include <util/stream/file.h>
#include <util/string/strip.h>
#include <util/system/env.h>

#include <userver/engine/run_standalone.hpp>

using namespace NLastGetopt;
using namespace NYdb;

namespace {

bool ParseToken(std::string& token, std::string& tokenFile) {
    if (!tokenFile.empty()) {
        if (!token.empty()) {
            Cerr << "Both token and token_file provided. Choose one." << Endl;
        } else {
            TFsPath path(tokenFile);
            if (path.Exists()) {
                token = Strip(TUnbufferedFileInput(path).ReadAll());
                return true;
            }
            Cerr << "Wrong path provided for token_file." << Endl;
        }
    } else if (!token.empty()) {
        return true;
    } else {
        token = GetEnv("YDB_TOKEN");
        return true;
    }
    return false;
}

void StartStatCollecting([[maybe_unused]] TDriver& driver, const std::string& statConfigFile) {
    if (statConfigFile.empty()) {
        return;
    }
}

} // namespace

// Override DoMain to wrap command dispatch in RunStandalone.
// The native DoMain (from slo-utils-base) runs commands directly in the calling
// thread. The userver version needs the coroutine engine running for
// ydb::TableClient, engine::Semaphore, AsyncNoSpan, SleepFor, WaitAny, etc.
int DoMain(int argc, char** argv, TCreateCommand create, TRunCommand run, TCleanupCommand cleanup) {
    TOpts opts = TOpts::Default();

    std::string connectionString;
    std::string prefix;
    std::string token;
    std::string tokenFile;
    std::string iamSaKeyFile;
    std::string statConfigFile;
    std::string balancingPolicy;

    opts.AddLongOption('c', "connection-string", "YDB connection string").Required().RequiredArgument("SCHEMA://HOST:PORT/?DATABASE=DATABASE")
        .StoreResult(&connectionString);
    opts.AddLongOption('p', "prefix", "Base prefix for tables").RequiredArgument("PATH")
        .StoreResult(&prefix);
    opts.AddLongOption('k', "token", "security token").RequiredArgument("TOKEN")
        .StoreResult(&token);
    opts.AddLongOption('f', "token-file", "security token file").RequiredArgument("PATH")
        .StoreResult(&tokenFile);
    opts.AddLongOption("iam-sa-key-file", "IAM service account key file").RequiredArgument("SECRET")
        .StoreResult(&iamSaKeyFile);
    opts.AddLongOption('s', "stat-config", "statistics config file").Optional().RequiredArgument("PATH")
        .StoreResult(&statConfigFile);
    opts.AddLongOption('b', "balancing-policy", "Balancing policy").Optional().DefaultValue("use-all-nodes").RequiredArgument("(use-all-nodes|prefer-local-dc|prefer-primary-pile)")
        .StoreResult(&balancingPolicy);
    opts.AddHelpOption('h');
    opts.SetFreeArgsMin(1);
    opts.SetFreeArgTitle(0, "<COMMAND>", GetCmdList());
    opts.ArgPermutation_ = NLastGetopt::REQUIRE_ORDER;

    TOptsParseResult res(&opts, argc, argv);
    size_t freeArgsPos = res.GetFreeArgsPos();
    argc -= freeArgsPos;
    argv += freeArgsPos;
    ECommandType command = ParseCommand(*argv);
    if (command == ECommandType::Unknown) {
        Cerr << "Unknown command '" << *argv << "'" << Endl;
        return EXIT_FAILURE;
    }

    if (prefix.empty()) {
        prefix = GetDatabase(connectionString);
    }

    if (!ParseToken(token, tokenFile)) {
        return EXIT_FAILURE;
    }

    auto config = TDriverConfig(connectionString);

    std::shared_ptr<NYdb::ICredentialsProviderFactory> credentials_provider_factory;
    if (!iamSaKeyFile.empty()) {
        Cout << "Enabling IAM authentication..." << Endl;
        credentials_provider_factory =
            CreateIamJwtFileCredentialsProviderFactory(TIamJwtFilename{.JwtFilename = iamSaKeyFile});
    } else if (!token.empty()) {
        Cout << "Enabling OAuth authentication..." << Endl;
        credentials_provider_factory = CreateOAuthCredentialsProviderFactory(token);
    } else {
        Cerr << "Warning: No authentication methods provided." << Endl;
    }

    if (credentials_provider_factory) {
        config.SetCredentialsProviderFactory(credentials_provider_factory);
    }

    if (balancingPolicy == "use-all-nodes") {
        config.SetBalancingPolicy(TBalancingPolicy::UseAllNodes());
    } else if (balancingPolicy == "prefer-local-dc") {
        config.SetBalancingPolicy(TBalancingPolicy::UsePreferableLocation());
    } else if (balancingPolicy == "prefer-primary-pile") {
        config.SetBalancingPolicy(TBalancingPolicy::UsePreferablePileState());
    } else {
        Cerr << "Unknown balancing policy: " << balancingPolicy << Endl;
        return EXIT_FAILURE;
    }

    const bool prefer_local_dc = balancingPolicy == "prefer-local-dc";

    TDriver driver(config);

    StartStatCollecting(driver, statConfigFile);

    TDatabaseOptions dbOptions{driver, prefix};
    int result = EXIT_FAILURE;

    try {
        // Wrap command execution in userver engine so coroutine-based
        // userver::ydb::TableClient and engine primitives work.
        // DDL operations (create table, drop table) use native SDK directly and
        // don't need the engine, but workload queries use userver::ydb::TableClient
        // which requires the engine to be running.
        userver::engine::RunStandalone(4, [&] {
            userver_slo::InitTableClient(
                config.GetEndpoint(),
                prefix,
                credentials_provider_factory,
                token,
                prefer_local_dc
            );
            switch (command) {
                case ECommandType::Create:
                    Cout << "Launching create command..." << Endl;
                    result = create(dbOptions, argc, argv);
                    break;
                case ECommandType::Run:
                    Cout << "Launching run command..." << Endl;
                    result = run(dbOptions, argc, argv);
                    break;
                case ECommandType::Cleanup:
                    Cout << "Launching cleanup command..." << Endl;
                    result = cleanup(dbOptions, argc);
                    break;
                default:
                    Cerr << "Unknown command" << Endl;
                    result = EXIT_FAILURE;
                    break;
            }
            // Destroy userver clients while the engine is still running.
            userver_slo::ShutdownTableClient();
        });
    } catch (const NYdb::NStatusHelpers::TYdbErrorException& e) {
        Cerr << "Exception caught: " << e << Endl;
        driver.Stop(true);
        return EXIT_FAILURE;
    }
    driver.Stop(true);
    return result;
}
