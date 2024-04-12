#include "uuid.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/str.h>
=======
#include <src/util/stream/str.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NKikimr {
namespace NUuid {

static void WriteHexDigit(ui8 digit, IOutputStream& out) {
    if (digit <= 9) {
        out << char('0' + digit);
    }
    else {
        out << char('a' + digit - 10);
    }
}

static void WriteHex(ui16 bytes, IOutputStream& out, bool reverseBytes = false) {
    if (reverseBytes) {
        WriteHexDigit((bytes >> 4) & 0x0f, out);
        WriteHexDigit(bytes & 0x0f, out);
        WriteHexDigit((bytes >> 12) & 0x0f, out);
        WriteHexDigit((bytes >> 8) & 0x0f, out);
    } else {
        WriteHexDigit((bytes >> 12) & 0x0f, out);
        WriteHexDigit((bytes >> 8) & 0x0f, out);
        WriteHexDigit((bytes >> 4) & 0x0f, out);
        WriteHexDigit(bytes & 0x0f, out);
    }
}

void UuidToString(ui16 dw[8], IOutputStream& out) {
    WriteHex(dw[1], out);
    WriteHex(dw[0], out);
    out << '-';
    WriteHex(dw[2], out);
    out << '-';
    WriteHex(dw[3], out);
    out << '-';
    WriteHex(dw[4], out, true);
    out << '-';
    WriteHex(dw[5], out, true);
    WriteHex(dw[6], out, true);
    WriteHex(dw[7], out, true);
}

void UuidHalfsToByteString(ui64 low, ui64 hi, IOutputStream& out) {
    union {
        char bytes[16];
        ui64 half[2];
    } buf;
    buf.half[0] = low;
    buf.half[1] = hi;
    out.Write(buf.bytes, 16);
}

}
}

