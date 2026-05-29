#include "key_value.h"

using namespace NLastGetopt;
using namespace NYdb;
using namespace NYdb::NTable;

namespace {

static bool DropTableWithRetry(TTableClient& client, const std::string& path) {
    try {
        RetryBackoff(client, 5, [path](TSession session) {
            TStatus status = session.DropTable(path).ExtractValueSync();
            if (status.GetStatus() == EStatus::NOT_FOUND) {
                return TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            }
            return status;
        });
        return true;
    } catch (const NYdb::NStatusHelpers::TYdbErrorException& e) {
        Cerr << "DropTable failed after retries: " << e << Endl;
        return false;
    }
}

} // namespace

int DropTable(TDatabaseOptions& dbOptions) {
    TTableClient client(dbOptions.Driver);
    if (!DropTableWithRetry(client, JoinPath(dbOptions.Prefix, TableName))) {
        return EXIT_FAILURE;
    }
    Cout << "Table dropped." << Endl;
    return EXIT_SUCCESS;
}
