#pragma once
#include <span>
#include "stack/stack_common.h"
#include "stack/stack.h"

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <src/util/system/context.h>
#include <ydb-cpp-sdk/util/system/defaults.h>
<<<<<<< HEAD
=======
#include <src/util/generic/noncopyable.h>
#include <src/util/generic/ptr.h>
#include <src/util/system/context.h>
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

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
        typedef std::function<void (TCont*)> TFunc;

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
