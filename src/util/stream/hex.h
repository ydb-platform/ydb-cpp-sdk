#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
=======
#include <src/util/system/types.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

class IOutputStream;

void HexEncode(const void* in, size_t len, IOutputStream& out);
void HexDecode(const void* in, size_t len, IOutputStream& out);
