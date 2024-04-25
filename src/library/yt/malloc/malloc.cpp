#include "malloc.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compiler.h>
#include <ydb-cpp-sdk/util/system/platform.h>
=======
#include <src/util/system/compiler.h>
#include <src/util/system/platform.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/compiler.h>
#include <ydb-cpp-sdk/util/system/platform.h>
>>>>>>> origin/main

#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////

extern "C" Y_WEAK size_t nallocx(size_t size, int /*flags*/) noexcept
{
    return size;
}

#ifndef _win_
extern "C" Y_WEAK size_t malloc_usable_size(void* /*ptr*/) noexcept
{
    return 0;
}
#endif

void* aligned_malloc(size_t size, size_t alignment)
{
#if defined(_win_)
    return _aligned_malloc(size, alignment);
#elif defined(_darwin_) || defined(_linux_)
    void* ptr = nullptr;
    ::posix_memalign(&ptr, alignment, size);
    return ptr;
#else
#   error Unsupported platform
#endif
}

////////////////////////////////////////////////////////////////////////////////
