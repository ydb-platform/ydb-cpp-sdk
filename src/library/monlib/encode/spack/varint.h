#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
=======
#include <src/util/system/types.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class IInputStream;
class IOutputStream;

namespace NMonitoring {
    ui32 WriteVarUInt32(IOutputStream* output, ui32 value);

    ui32 ReadVarUInt32(IInputStream* input);
    size_t ReadVarUInt32(const ui8* buf, size_t len, ui32* result);

    enum class EReadResult {
        OK,
        ERR_OVERFLOW,
        ERR_UNEXPECTED_EOF,
    };

    [[nodiscard]]
    EReadResult TryReadVarUInt32(IInputStream* input, ui32* value);

}
