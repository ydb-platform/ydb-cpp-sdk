#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/memory/alloc.h>

#include <new>
#include <cstdlib>

void TFree::DoDestroy(void* t) noexcept {
    free(t);
}

void TDelete::Destroy(void* t) noexcept {
    ::operator delete(t);
}

TThrRefBase::~TThrRefBase() = default;
