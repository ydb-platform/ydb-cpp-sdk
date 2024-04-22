#include <ydb-cpp-sdk/library/http/io/stream.h>
#include <ydb-cpp-sdk/util/stream/output.h>

int main() {
    for (auto codec : SupportedCodings()) {
        std::cout << codec << std::endl;
    }
}
