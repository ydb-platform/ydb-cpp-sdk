#include <ydb-cpp-sdk/util/memory/alloc.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/singleton.h>
=======
#include <src/util/generic/singleton.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/singleton.h>
>>>>>>> origin/main
#include <src/util/system/sys_alloc.h>

using TBlock = TDefaultAllocator::TBlock;

TBlock TDefaultAllocator::Allocate(size_t len) {
    TBlock ret = {y_allocate(len), len};

    return ret;
}

void TDefaultAllocator::Release(const TBlock& block) {
    y_deallocate(block.Data);
}

IAllocator* TDefaultAllocator::Instance() noexcept {
    return SingletonWithPriority<TDefaultAllocator, 0>();
}
