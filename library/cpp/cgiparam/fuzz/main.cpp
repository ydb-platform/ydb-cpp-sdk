#include <library/cpp/cgiparam/cgiparam.h>

extern "C" int LLVMFuzzerTestOneInput(const ui8* data, size_t size) {
    try {
        TCgiParameters(std::string_view((const char*)data, size));
    } catch (...) {
        // ¯\_(ツ)_/¯
    }

    return 0; // Non-zero return values are reserved for future use.
}
