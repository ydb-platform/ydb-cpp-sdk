#include <src/library/monlib/encode/json/json.h>
#include <src/library/monlib/encode/fake/fake.h>

#include <string_view>


extern "C" int LLVMFuzzerTestOneInput(const ui8* data, size_t size) {
    auto encoder = NMonitoring::EncoderFake();

    try {
        NMonitoring::DecodeJson({reinterpret_cast<const char*>(data), size}, encoder.Get());
    } catch (...) {
    }

    return 0;
}
