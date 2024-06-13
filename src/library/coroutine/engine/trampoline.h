#pragma once

#include "stack/stack_common.h"
#include "stack/stack.h"

#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/system/defaults.h>

#include <src/util/system/context.h>

#include <functional>
#include <span>

#if !defined(STACK_GROW_DOWN)
#   error "unsupported"
#endif

class TCont;
typedef void (*TContFunc)(TCont*, void*);

namespace NCoro {

    namespace NStack {
        class IAllocator;
    }

    class TTrampoline : public ITrampoLine, TNonCopyable {
    public:
        using TFunc = std::function<void (TCont*)>;

        TTrampoline(
            NCoro::NStack::IAllocator& allocator,
            uint32_t stackSize,
            TFunc f,
            TCont* cont
        ) noexcept;

        std::span<char> Stack() noexcept;

        TExceptionSafeContext* Context() noexcept {
            return &Ctx_;
        }

        void SwitchTo(TExceptionSafeContext* ctx) noexcept {
            Y_ABORT_UNLESS(Stack_.LowerCanaryOk(), "Stack overflow (%s)", ContName());
            Y_ABORT_UNLESS(Stack_.UpperCanaryOk(), "Stack override (%s)", ContName());
            Ctx_.SwitchTo(ctx);
        }

        void DoRun() override;

        void DoRunNaked() override;

    private:
        const char* ContName() const noexcept;
    private:
        NStack::TStackHolder Stack_;
        const TContClosure Clo_;
        TExceptionSafeContext Ctx_;
        TFunc Func_;
        TCont* const Cont_;
    };
}
