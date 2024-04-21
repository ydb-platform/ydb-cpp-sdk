#include <ydb-cpp-sdk/library/threading/future/core/future.h>

namespace NThreading::NImpl {
    [[noreturn]] void ThrowFutureException(std::string_view message, const TSourceLocation& source) {
        throw source + TFutureException() << message;
    }
}
