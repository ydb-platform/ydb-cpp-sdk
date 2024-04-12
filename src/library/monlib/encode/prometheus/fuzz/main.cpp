#include <src/library/monlib/encode/prometheus/prometheus.h>
#include <src/library/monlib/encode/fake/fake.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/mem.h>
=======
#include <src/util/stream/mem.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))


extern "C" int LLVMFuzzerTestOneInput(const ui8* buf, size_t size) {
    using namespace NMonitoring;

    try {
        std::string_view data(reinterpret_cast<const char*>(buf), size);
        auto encoder = EncoderFake();
        DecodePrometheus(data, encoder.Get());
    } catch (...) {
    }

    return 0;
}
