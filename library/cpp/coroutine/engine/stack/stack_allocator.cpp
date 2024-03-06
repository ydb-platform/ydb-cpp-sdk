#include "stack_allocator.h"


namespace NCoro::NStack {

    std::unique_ptr<IAllocator> GetAllocator(std::optional<TPoolAllocatorSettings> poolSettings, EGuard guardType) {
        std::unique_ptr<IAllocator> allocator;
        if (poolSettings) {
            if (guardType == EGuard::Canary) {
                allocator = std::make_unique<TPoolAllocator<TCanaryGuard>>(*poolSettings);
            } else {
                Y_ASSERT(guardType == EGuard::Page);
                allocator = std::make_unique<TPoolAllocator<TPageGuard>>(*poolSettings);
            }
        } else {
            if (guardType == EGuard::Canary) {
                allocator = std::make_unique<TSimpleAllocator<TCanaryGuard>>();
            } else {
                Y_ASSERT(guardType == EGuard::Page);
                allocator = std::make_unique<TSimpleAllocator<TPageGuard>>();
            }
        }
        return allocator;
    }

}