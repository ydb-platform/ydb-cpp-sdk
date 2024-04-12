<<<<<<< HEAD
#include <ydb-cpp-sdk/library/http/io/stream.h>
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/library/http/io/stream.h>
#include <src/util/stream/output.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

int main() {
    for (auto codec : SupportedCodings()) {
        std::cout << codec << std::endl;
    }
}
