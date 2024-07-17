#include "stack_allocator.h"
#include "stack_common.h"

#include <src/library/testing/gtest/gtest.h>


using namespace testing;

namespace NCoro::NStack::Tests {

    enum class EAllocator {
        Pool,    // allocates page-size aligned stacks from pools
        Simple  // uses malloc/free for each stack
    };

    class TAllocatorParamFixture : public TestWithParam< std::tuple<EGuard, EAllocator> > {
    protected: // methods
    void SetUp() override {
        EGuard guardType;
        EAllocator allocType;
        std::tie(guardType, allocType) = GetParam();

        std::optional<TPoolAllocatorSettings> poolSettings;
        if (allocType == EAllocator::Pool) {
            poolSettings = TPoolAllocatorSettings{};
        }

        Allocator_ = GetAllocator(poolSettings, guardType);
    }

    protected: // data
        std::unique_ptr<IAllocator> Allocator_;
    };


    TEST_P(TAllocatorParamFixture, StackAllocationAndRelease) {
        size_t stackSize = PageSize * 12;
        auto stack = Allocator_->AllocStack(stackSize, "test_stack");
#if defined(_san_enabled_) || !defined(NDEBUG)
        stackSize *= DebugOrSanStackMultiplier;
#endif

        // Correct stack should have
        EXPECT_EQ(stack.GetSize(), stackSize); // predefined size
        EXPECT_FALSE(reinterpret_cast<size_t>(stack.GetAlignedMemory()) & PageSizeMask); // aligned pointer
        // Writable workspace
        auto workspace = Allocator_->GetStackWorkspace(stack.GetAlignedMemory(), stack.GetSize());
        for (size_t i = 0; i < workspace.size(); i += 512) {
            workspace[i] = 42;
        }
        EXPECT_TRUE(Allocator_->CheckStackOverflow(stack.GetAlignedMemory()));
        EXPECT_TRUE(Allocator_->CheckStackOverride(stack.GetAlignedMemory(), stack.GetSize()));

        Allocator_->FreeStack(stack);
        EXPECT_FALSE(stack.GetRawMemory());
    }

    INSTANTIATE_TEST_SUITE_P(AllocatorTestParams, TAllocatorParamFixture,
            Combine(Values(EGuard::Canary, EGuard::Page), Values(EAllocator::Pool, EAllocator::Simple)));


    // ------------------------------------------------------------------------
    // Test that allocated stack has guards
    //
    template<class AllocatorType>
    std::unique_ptr<IAllocator> GetAllocator(EGuard guardType);

    struct TPoolTag {};
    struct TSimpleTag {};

    template<>
    std::unique_ptr<IAllocator> GetAllocator<TPoolTag>(EGuard guardType) {
        std::optional<TPoolAllocatorSettings> poolSettings = TPoolAllocatorSettings{};
        return GetAllocator(poolSettings, guardType);
    }

    template<>
    std::unique_ptr<IAllocator> GetAllocator<TSimpleTag>(EGuard guardType) {
        std::optional<TPoolAllocatorSettings> poolSettings;
        return GetAllocator(poolSettings, guardType);
    }


    template <class AllocatorType>
    class TAllocatorFixture : public Test {
    protected:
        TAllocatorFixture()
            : Allocator_(GetAllocator<AllocatorType>(EGuard::Page))
        {}

        const size_t StackSize_ = PageSize * 2;
        std::unique_ptr<IAllocator> Allocator_;
    };

    using Implementations = Types<TPoolTag, TSimpleTag>;
    TYPED_TEST_SUITE(TAllocatorFixture, Implementations);

    TYPED_TEST(TAllocatorFixture, StackOverflow) {
        ASSERT_DEATH({
            auto stack = this->Allocator_->AllocStack(this->StackSize_, "test_stack");

            // Overwrite previous guard, crash is here
            *(stack.GetAlignedMemory() - 1) = 42;
        }, "");
    }

    TYPED_TEST(TAllocatorFixture, StackOverride) {
        ASSERT_DEATH({
            auto stack = this->Allocator_->AllocStack(this->StackSize_, "test_stack");

            // Overwrite guard, crash is here
            *(stack.GetAlignedMemory() + stack.GetSize() - 1) = 42;
        }, "");
    }

}
