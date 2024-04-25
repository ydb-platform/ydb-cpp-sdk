#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
=======
#include <src/util/system/types.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/types.h>
>>>>>>> origin/main

class IOutputStream;

void HexEncode(const void* in, size_t len, IOutputStream& out);
void HexDecode(const void* in, size_t len, IOutputStream& out);
