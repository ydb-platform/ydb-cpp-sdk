#include <cstdlib>
#include <exception>

#include <util/stream/output.h>
#include <util/system/backtrace.h>
#include <util/generic/yexception.h>

#include <iostream>
#include <string>

namespace {
    // Avoid infinite recursion if std::terminate is triggered anew by the
    // FancyTerminateHandler.
    thread_local int TerminateCount = 0;

    void FancyTerminateHandler() {
        switch (++TerminateCount) {
            case 1:
                break;
            case 2:
                std::cerr << "FancyTerminateHandler called recursively" << std::endl;
                [[fallthrough]];
            default:
                abort();
                break;
        }

        if (std::current_exception()) {
            std::cerr << "Uncaught exception: " << CurrentExceptionMessage() << '\n';
        } else {
            std::cerr << "Terminate for unknown reason (no current exception)\n";
        }
        PrintBackTrace();
        std::cerr.flush();
        abort();
    }

    [[maybe_unused]] auto _ = std::set_terminate(&FancyTerminateHandler);
}
