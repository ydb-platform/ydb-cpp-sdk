#include "hex.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include "output.h"
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/stream/output.h>
>>>>>>> origin/main
#include <src/util/string/hex.h>

void HexEncode(const void* in, size_t len, IOutputStream& out) {
    static const size_t NUM_OF_BYTES = 32;
    char buffer[NUM_OF_BYTES * 2];

    auto current = static_cast<const char*>(in);
    for (size_t take = 0; len; current += take, len -= take) {
        take = Min(NUM_OF_BYTES, len);
        HexEncode(current, take, buffer);
        out.Write(buffer, take * 2);
    }
}

void HexDecode(const void* in, size_t len, IOutputStream& out) {
    Y_ENSURE(!(len & 1), std::string_view("Odd buffer length passed to HexDecode"));

    static const size_t NUM_OF_BYTES = 32;
    char buffer[NUM_OF_BYTES];

    auto current = static_cast<const char*>(in);
    for (size_t take = 0; len; current += take, len -= take) {
        take = Min(NUM_OF_BYTES * 2, len);
        HexDecode(current, take, buffer);
        out.Write(buffer, take / 2);
    }
}
