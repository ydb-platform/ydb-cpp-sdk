#include "handlers.h"
#include <client/ydb_types/exceptions/exceptions.h>

namespace NYdb {

void ThrowFatalError(const std::string& str) {
    throw TContractViolation(str);
}

} // namespace NYdb
