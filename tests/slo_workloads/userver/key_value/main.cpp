#include "userver_utils.h"
#include "key_value.h"

int main(int argc, char** argv) {
    // userver::ydb::TableClient does not expose its per-request retry count.
    return DoMain(argc, argv, DoCreate, DoRun, DoCleanup, false, true);
}
