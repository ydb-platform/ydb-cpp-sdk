<<<<<<< HEAD
#include <ydb-cpp-sdk/library/http/io/stream.h>


#include <ydb-cpp-sdk/util/stream/mem.h>
=======
#include <src/library/http/io/stream.h>


#include <src/util/stream/mem.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

extern "C" int LLVMFuzzerTestOneInput(const ui8* data, size_t size) {
    TMemoryInput mi(data, size);

    try {
        THttpInput(&mi).ReadAll();
    } catch (...) {
    }

    return 0; // Non-zero return values are reserved for future use.
}
