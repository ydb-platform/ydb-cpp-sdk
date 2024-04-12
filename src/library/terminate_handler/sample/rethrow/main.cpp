<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

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
