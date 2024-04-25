<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main

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
