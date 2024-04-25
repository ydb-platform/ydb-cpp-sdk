#include <src/library/monlib/encode/spack/spack_v1.h>
#include <src/library/monlib/encode/fake/fake.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/mem.h>
=======
#include <src/util/stream/mem.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/stream/mem.h>
>>>>>>> origin/main


extern "C" int LLVMFuzzerTestOneInput(const ui8* data, size_t size) {
    using namespace NMonitoring;

    TMemoryInput min{data, size};

    auto encoder = EncoderFake();

    try {
        DecodeSpackV1(&min, encoder.Get());
    } catch (...) {
    }

    return 0;
}
