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

#include <vector>

using namespace NLastGetopt;
using namespace NYdb;

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

    std::string defaultConnectionString = DefaultConnectionStringFromEnv();

    auto& connOpt = opts.AddLongOption('c', "connection-string", "YDB connection string").RequiredArgument("SCHEMA://HOST:PORT/?DATABASE=DATABASE")
        .StoreResult(&connectionString);
    if (!defaultConnectionString.empty()) {
        connOpt.DefaultValue(defaultConnectionString);
    } else {
        connOpt.Required();
    }
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
    opts.SetFreeArgsMin(0);
    opts.SetFreeArgTitle(0, "<COMMAND>", GetCmdList());
    opts.ArgPermutation_ = NLastGetopt::REQUIRE_ORDER;
    // Run-phase options (--read-rps, --write-rps, …) reach DoMain when the
    // caller invokes the workload without an explicit subcommand (the v2 SLO
    // action contract). Tolerate them here so the global parser stops at the
    // first unknown option instead of erroring; they are forwarded to the
    // run phase below.
    opts.AllowUnknownLongOptions_ = true;

    TOptsParseResult res(&opts, argc, argv);
    size_t freeArgsPos = res.GetFreeArgsPos();
    argc -= freeArgsPos;
    argv += freeArgsPos;

    ECommandType command = (argc > 0) ? ParseCommand(*argv) : ECommandType::All;
    if (command == ECommandType::Unknown) {
        if (argv[0][0] == '-') {
            command = ECommandType::All;
        } else {
            Cerr << "Unknown command '" << *argv << "'" << Endl;
            return EXIT_FAILURE;
        }
    }

    if (prefix.empty()) {
        prefix = GetDatabase(connectionString);
    }
    if (prefix.empty()) {
        prefix = GetEnv("YDB_DATABASE");
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
        // DDL operations (create table, drop table) use native SDK directly
        // and don't need the engine, but workload queries use
        // userver::ydb::TableClient which requires the engine to be running.
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
                case ECommandType::All: {
                    Cout << "Launching full lifecycle: create -> run -> cleanup" << Endl;
                    // Forward leftover argv to the run phase so options
                    // like --read-rps / --write-rps take effect. argv[0]
                    // here is the first run-phase option (no subcommand
                    // keyword was supplied), so prepend a synthetic
                    // program name for ParseOptionsRun.
                    char programName[] = "slo";
                    std::vector<char*> runArgv;
                    runArgv.reserve(argc + 2);
                    runArgv.push_back(programName);
                    for (int i = 0; i < argc; ++i) {
                        runArgv.push_back(argv[i]);
                    }
                    runArgv.push_back(nullptr);
                    int fakeArgc = 1;
                    char* fakeArgv[] = { programName, nullptr };

                    Cout << "[all] Launching create command..." << Endl;
                    result = create(dbOptions, fakeArgc, fakeArgv);
                    if (!result) {
                        Cout << "[all] Launching run command..." << Endl;
                        result = run(dbOptions, static_cast<int>(runArgv.size() - 1), runArgv.data());
                    }
                    Cout << "[all] Launching cleanup command..." << Endl;
                    int cleanupRc = cleanup(dbOptions, fakeArgc);
                    // Cleanup runs while chaos-monkey is still killing
                    // nodes, so a DropTable failure here is expected noise
                    // and must not mask a successful run. Surface the
                    // run's status; only fall back to cleanup status when
                    // run itself failed.
                    if (cleanupRc && !result) {
                        Cerr << "[all] Warning: cleanup failed (exit " << cleanupRc
                            << ") but run succeeded; ignoring cleanup exit code." << Endl;
                    } else if (cleanupRc) {
                        Cerr << "[all] Warning: cleanup failed (exit " << cleanupRc
                            << "); preserving earlier run failure." << Endl;
                    }
                    break;
                }
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
