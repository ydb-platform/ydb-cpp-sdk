#include <ydb-cpp-sdk/util/generic/flags.h>

#include <src/util/stream/format.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/yassert.h>
=======
#include <src/util/system/yassert.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/yassert.h>
>>>>>>> origin/main

void ::NPrivate::PrintFlags(IOutputStream& stream, ui64 value, size_t size) {
    /* Note that this function is in cpp because we need to break circular
     * dependency between TFlags and ENumberFormat. */
    stream << "TFlags(";

    switch (size) {
        case 1:
            stream << Bin(static_cast<ui8>(value), HF_FULL);
            break;
        case 2:
            stream << Bin(static_cast<ui16>(value), HF_FULL);
            break;
        case 4:
            stream << Bin(static_cast<ui32>(value), HF_FULL);
            break;
        case 8:
            stream << Bin(static_cast<ui64>(value), HF_FULL);
            break;
        default:
            Y_ABORT_UNLESS(false);
    }

    stream << ")";
}
