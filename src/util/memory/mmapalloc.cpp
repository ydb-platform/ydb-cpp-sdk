#include <ydb-cpp-sdk/util/memory/alloc.h>
#include "mmapalloc.h"

#include <src/util/system/filemap.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>

namespace {
    class TMmapAllocator: public IAllocator {
    public:
        TBlock Allocate(size_t len) override {
            TMappedAllocation m(len + sizeof(TMappedAllocation));
            TMappedAllocation* real = (TMappedAllocation*)m.Data();

            (new (real) TMappedAllocation(0))->swap(m);

            TBlock ret = {real + 1, len};

            return ret;
        }

        void Release(const TBlock& block) override {
            TMappedAllocation tmp(0);
            TMappedAllocation* real = ((TMappedAllocation*)block.Data) - 1;

            real->swap(tmp);
            real->~TMappedAllocation();
        }
    };
}

IAllocator* MmapAllocator() {
    return SingletonWithPriority<TMmapAllocator, 0>();
}
