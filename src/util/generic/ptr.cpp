#include <ydb-cpp-sdk/util/generic/ptr.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/memory/alloc.h>
=======
#include <src/util/system/defaults.h>
#include <src/util/memory/alloc.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/memory/alloc.h>
>>>>>>> origin/main

#include <new>
#include <cstdlib>

void TFree::DoDestroy(void* t) noexcept {
    free(t);
}

void TDelete::Destroy(void* t) noexcept {
    ::operator delete(t);
}

TThrRefBase::~TThrRefBase() = default;
