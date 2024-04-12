#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/system/defaults.h>
=======
#include <src/util/stream/input.h>
#include <src/util/stream/output.h>
#include <src/util/system/defaults.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    // Various functions that read/write varints from/to a stream.

    // Returns the number of bytes written.
    int WriteVarUInt64(IOutputStream* output, ui64 value);
    int WriteVarInt32(IOutputStream* output, i32 value);
    int WriteVarInt64(IOutputStream* output, i64 value);

    // Returns the number of bytes read.
    int ReadVarUInt64(IInputStream* input, ui64* value);
    int ReadVarInt32(IInputStream* input, i32* value);
    int ReadVarInt64(IInputStream* input, i64* value);

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
