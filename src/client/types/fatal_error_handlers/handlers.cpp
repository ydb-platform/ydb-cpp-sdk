#include "handlers.h"
#include <src/client/types/exceptions/exceptions.h>

namespace NYdb {

void ThrowFatalError(const std::string& str) {
    throw TContractViolation(str);
}

} // namespace NYdb
