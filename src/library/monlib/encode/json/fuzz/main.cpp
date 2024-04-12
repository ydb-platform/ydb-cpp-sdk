<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/encode/json/json.h>
=======
#include <src/library/monlib/encode/json/json.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
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
