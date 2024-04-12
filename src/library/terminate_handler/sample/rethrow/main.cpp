#include <src/util/generic/yexception.h>

#include <iostream>

void Bar() {
    ythrow yexception() << "from Foo()";
}

void Foo() {
    try {
        Bar();
    } catch (...) {
        std::cerr << "caught; rethrowing\n";
        throw;
    }
}

int main() {
    Foo();
    return 0;
}
