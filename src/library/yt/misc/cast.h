#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/yt/exception/exception.h>
=======
#include <src/library/yt/exception/exception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

template <class T, class S>
bool TryIntegralCast(S value, T* result);

template <class T, class S>
T CheckedIntegralCast(S value);

////////////////////////////////////////////////////////////////////////////////

template <class T, class S>
bool TryEnumCast(S value, T* result);

template <class T, class S>
T CheckedEnumCast(S value);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

#define CAST_INL_H_
#include "cast-inl.h"
#undef CAST_INL_H_
