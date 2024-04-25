#pragma once
#ifndef MEMORY_TAG_INL_H_
#error "Direct inclusion of this file is not allowed, include memory_tag.h"
// For the sake of sane code completion.
#include "memory_tag.h"
#endif

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


inline TMemoryTagGuard::TMemoryTagGuard()
    : Active_(false)
    , PreviousTag_(NullMemoryTag)
{ }

inline TMemoryTagGuard::TMemoryTagGuard(TMemoryTag tag)
    : Active_(true)
    , PreviousTag_(GetCurrentMemoryTag())
{
    SetCurrentMemoryTag(tag);
}

inline TMemoryTagGuard::TMemoryTagGuard(TMemoryTagGuard&& other)
    : Active_(other.Active_)
    , PreviousTag_(other.PreviousTag_)
{
    other.Active_ = false;
    other.PreviousTag_ = NullMemoryTag;
}

inline TMemoryTagGuard::~TMemoryTagGuard()
{
    if (Active_) {
        SetCurrentMemoryTag(PreviousTag_);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
