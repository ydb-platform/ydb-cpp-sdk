#include "stack_allocator.h"


namespace NCoro::NStack {

    std::unique_ptr<IAllocator> GetAllocator(std::optional<TPoolAllocatorSettings> poolSettings, EGuard guardType) {
        std::unique_ptr<IAllocator> allocator;
        if (poolSettings) {
            if (guardType == EGuard::Canary) {
                allocator = MakeHolder<TPoolAllocator<TCanaryGuard>>(*poolSettings);
            } else {
                Y_ASSERT(guardType == EGuard::Page);
                allocator = MakeHolder<TPoolAllocator<TPageGuard>>(*poolSettings);
            }
        } else {
            if (guardType == EGuard::Canary) {
                allocator = MakeHolder<TSimpleAllocator<TCanaryGuard>>();
            } else {
                Y_ASSERT(guardType == EGuard::Page);
                allocator = MakeHolder<TSimpleAllocator<TPageGuard>>();
            }
        }
        return allocator;
    }

}