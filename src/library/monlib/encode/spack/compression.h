#pragma once

#include "spack_v1.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/input.h>
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NMonitoring {

class IFramedCompressStream: public IOutputStream {
public:
    virtual void FlushWithoutEmptyFrame() = 0;
    virtual void FinishAndWriteEmptyFrame() = 0;
};

THolder<IInputStream> CompressedInput(IInputStream* in, ECompression alg);
THolder<IFramedCompressStream> CompressedOutput(IOutputStream* out, ECompression alg);

} // namespace NMonitoring
