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

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

// These Functions provide coding of integers with property: 0 <= f(x) <= 2 * |x|
// Actually taken 'as is' from protobuf/wire_format_lite.h

ui32 ZigZagEncode32(i32 n);
i32 ZigZagDecode32(ui32 n);

ui64 ZigZagEncode64(i64 n);
i64 ZigZagDecode64(ui64 n);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

#define ZIG_ZAG_INL_H_
#include "zig_zag-inl.h"
#undef ZIG_ZAG_INL_H_
