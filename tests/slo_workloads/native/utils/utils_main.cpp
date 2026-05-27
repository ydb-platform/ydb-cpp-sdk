#include "utils.h"

#include <ydb-cpp-sdk/client/iam/iam.h>

#include <util/folder/path.h>
#include <util/stream/file.h>
#include <util/string/strip.h>
#include <util/system/env.h>

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
