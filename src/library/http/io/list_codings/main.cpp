#include <ydb-cpp-sdk/library/http/io/stream.h>
#include <src/util/stream/output.h>

int main() {
    for (auto codec : SupportedCodings()) {
        std::cout << codec << std::endl;
    }
}
