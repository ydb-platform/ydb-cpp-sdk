#include "utils.h"

#include <ydb-cpp-sdk/client/iam/iam.h>

#include <util/folder/path.h>
#include <util/stream/file.h>
#include <util/string/builder.h>
#include <util/string/strip.h>
#include <util/system/env.h>

#include <vector>

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

std::string DefaultConnectionStringFromEnv() {
    std::string cs = GetEnv("YDB_CONNECTION_STRING");
    if (!cs.empty()) {
        return cs;
    }
    std::string endpoint = GetEnv("YDB_ENDPOINT");
    std::string database = GetEnv("YDB_DATABASE");
    if (!endpoint.empty() && !database.empty()) {
        return TStringBuilder() << endpoint << "/?database=" << database;
    }
    return {};
}

} // namespace

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
        if (argc > 0 && argv[0][0] == '-') {
            // First leftover token is an option, not a subcommand keyword:
            // treat as implicit All mode and let the run phase parse it.
            command = ECommandType::All;
        } else if (argc > 0) {
            Cerr << "Unknown command '" << *argv << "'" << Endl;
            return EXIT_FAILURE;
        } else {
            command = ECommandType::All;
        }
    }

    if (prefix.empty()) {
        prefix = GetDatabase(connectionString);
    }
    if (prefix.empty()) {
        // YDB SLO action sets YDB_CONNECTION_STRING in path form
        // (grpc://host:port/Root/testdb), which GetDatabase can't parse.
        // Fall back to YDB_DATABASE which the action sets alongside it.
        prefix = GetEnv("YDB_DATABASE");
    }

    if (!ParseToken(token, tokenFile)) {
        return EXIT_FAILURE;
    }

    auto config = TDriverConfig(connectionString);

    if (!iamSaKeyFile.empty()) {
        Cout << "Enabling IAM authentication..." << Endl;
        TIamJwtFilename iamJwtFilename{.JwtFilename = iamSaKeyFile};
        config.SetCredentialsProviderFactory(CreateIamJwtFileCredentialsProviderFactory(iamJwtFilename));
    } else if (!token.empty()) {
        Cout << "Enabling OAuth authentication..." << Endl;
        config.SetCredentialsProviderFactory(CreateOAuthCredentialsProviderFactory(token));
    } else {
        Cerr << "Warning: No authentication methods provided." << Endl;
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

    TDriver driver(config);

    StartStatCollecting(driver, statConfigFile);

    TDatabaseOptions dbOptions{driver, prefix};
    int result;
    try {
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
                return EXIT_FAILURE;
        }
    } catch (const NYdb::NStatusHelpers::TYdbErrorException& e) {
        Cerr << "Exception caught: " << e << Endl;
        return EXIT_FAILURE;
    }
    driver.Stop(true);
    return result;
}
