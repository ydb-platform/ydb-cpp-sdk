<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))


void Foo(unsigned i = 0) {
    if (i >= 10) {
        ythrow yexception() << "from Foo()";
    } else {
        Foo(i + 1);
    }
}

int main() {
    Foo();
    return 0;
}
