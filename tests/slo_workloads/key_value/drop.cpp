#include "key_value.h"

using namespace NLastGetopt;
using namespace NYdb;
using namespace NYdb::NTable;

int DropTable(TDatabaseOptions& dbOptions) {
    TTableClient client(dbOptions.Driver);
    const std::string path = JoinPath(dbOptions.Prefix, TableName);
    TStatus status = client.RetryOperationSync([path](TSession session) {
        TStatus dropStatus = session.DropTable(path).ExtractValueSync();
        if (dropStatus.GetStatus() == EStatus::NOT_FOUND) {
            return TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
        }
        return dropStatus;
    });
    if (!status.IsSuccess()) {
        Cerr << "DropTable failed: " << status << Endl;
        return EXIT_FAILURE;
    }
    Cout << "Table dropped." << Endl;
    return EXIT_SUCCESS;
}
