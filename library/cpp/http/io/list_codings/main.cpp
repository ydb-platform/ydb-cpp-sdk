#include <library/cpp/http/io/stream.h>

#include <iostream>

int main() {
    for (auto codec : SupportedCodings()) {
        std::cout << codec << std::endl;
    }
}
