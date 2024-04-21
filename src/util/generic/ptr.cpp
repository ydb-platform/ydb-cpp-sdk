#include "ptr.h"

#include <ydb-cpp-sdk/util/system/defaults.h>
#include <src/util/memory/alloc.h>

#include <new>
#include <cstdlib>

void TDelete::Destroy(void* t) noexcept {
    ::operator delete(t);
}

TThrRefBase::~TThrRefBase() = default;
