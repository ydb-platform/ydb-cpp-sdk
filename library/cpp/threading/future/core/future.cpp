#include "future.h"

namespace NThreading::NImpl {
    [[noreturn]] void ThrowFutureException(std::string_view message, const TSourceLocation& source) {
        throw source + TFutureException() << message;
    }
}
