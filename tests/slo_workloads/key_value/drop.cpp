#include "key_value.h"

using namespace NLastGetopt;
using namespace NYdb;
using namespace NYdb::NTable;

namespace {

static bool DropTableWithRetry(TTableClient& client, const std::string& path) {
    TDuration delay = TDuration::Seconds(1);
    constexpr std::uint32_t kMaxAttempts = 3;

    for (std::uint32_t attempt = 1; attempt <= kMaxAttempts; ++attempt) {
        TStatus status = client.RetryOperationSync([path](TSession session) {
            TStatus dropStatus = session.DropTable(path).ExtractValueSync();
            if (dropStatus.GetStatus() == EStatus::NOT_FOUND) {
                return TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            }
            return dropStatus;
        });
        if (status.IsSuccess()) {
            return true;
        }
        Cerr << "DropTable attempt " << attempt << " failed: " << status << Endl;
        if (attempt == kMaxAttempts) {
            break;
        }
        Sleep(delay);
        delay = Min(delay * 2, TDuration::Seconds(8));
    }
    return false;
}

} // namespace

int DropTable(TDatabaseOptions& dbOptions) {
    TTableClient client(dbOptions.Driver);
    if (!DropTableWithRetry(client, JoinPath(dbOptions.Prefix, TableName))) {
        Cerr << "DropTable failed after all retries." << Endl;
        return EXIT_FAILURE;
    }
    Cout << "Table dropped." << Endl;
    return EXIT_SUCCESS;
}
