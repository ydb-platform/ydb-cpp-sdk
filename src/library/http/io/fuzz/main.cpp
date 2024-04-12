#include <src/library/http/io/stream.h>


#include <src/util/stream/mem.h>

extern "C" int LLVMFuzzerTestOneInput(const ui8* data, size_t size) {
    TMemoryInput mi(data, size);

    try {
        THttpInput(&mi).ReadAll();
    } catch (...) {
    }

    return 0; // Non-zero return values are reserved for future use.
}
