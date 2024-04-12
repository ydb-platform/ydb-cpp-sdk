#include <cstdlib>
#include <exception>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/system/backtrace.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/stream/output.h>
#include <src/util/system/backtrace.h>
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
