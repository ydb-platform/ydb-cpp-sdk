#pragma once

#include "stack.h"
#include "stack_common.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/noncopyable.h>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
>>>>>>> origin/main

#include <span>
#include <cstdint>
#include <optional>

namespace NCoro::NStack {

    class IAllocator : private TNonCopyable {
    public:
        virtual ~IAllocator() = default;

        //! Size should be page-aligned. Stack would be protected by guard, thus, actual
        //! workspace for stack = size - size of guard.
        NDetails::TStack AllocStack(size_t size, const char* name) {
            size_t alignedSize = (size + PageSize - 1) & ~PageSizeMask;
            Y_ASSERT(alignedSize < 10 * 1024 * PageSize); // more than 10K pages for stack - do you really need it?
#if defined(_san_enabled_) || !defined(NDEBUG)
            alignedSize *= DebugOrSanStackMultiplier;
#endif
            return DoAllocStack(alignedSize, name);
        }

        void FreeStack(NDetails::TStack& stack) noexcept {
            if (stack.GetAlignedMemory()) {
                DoFreeStack(stack);
            }
        }

        virtual TAllocatorStats GetStackStats() const noexcept = 0;

        // Stack helpers
        virtual std::span<char> GetStackWorkspace(void* stack, size_t size) noexcept = 0;
        virtual bool CheckStackOverflow(void* stack) const noexcept = 0;
        virtual bool CheckStackOverride(void* stack, size_t size) const noexcept = 0;

    private:
        virtual NDetails::TStack DoAllocStack(size_t size, const char* name) = 0;
        virtual void DoFreeStack(NDetails::TStack& stack) noexcept = 0;
    };

    THolder<IAllocator> GetAllocator(std::optional<TPoolAllocatorSettings> poolSettings, EGuard guardType);

}

#include "stack_allocator.inl"
