#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compiler.h>
#include <ydb-cpp-sdk/util/system/defaults.h>
=======
#include <src/util/system/compiler.h>
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/compiler.h>
#include <ydb-cpp-sdk/util/system/defaults.h>
>>>>>>> origin/main

#include <utility>

namespace NPrivate {
    template <typename F>
    class TScopeGuard {
    public:
        TScopeGuard(const F& function)
            : Function_{function}
        {
        }

        TScopeGuard(F&& function)
            : Function_{std::move(function)}
        {
        }

        TScopeGuard(TScopeGuard&&) = default;
        TScopeGuard(const TScopeGuard&) = default;

        ~TScopeGuard() {
            Function_();
        }

    private:
        F Function_;
    };

    struct TMakeGuardHelper {
        template <class F>
        TScopeGuard<F> operator|(F&& function) const {
            return std::forward<F>(function);
        }
    };
}

// \brief `Y_SCOPE_EXIT(captures) { body };`
//
// General implementaion of RAII idiom (resource acquisition is initialization). Executes
// function upon return from the current scope.
//
// @note expects `body` to provide no-throw guarantee, otherwise whenever an exception
// is thrown and leaves the outermost block of `body`, the function `std::terminate` is called.
// @see http://drdobbs.com/184403758 for detailed motivation.
#define Y_SCOPE_EXIT(...) const auto Y_GENERATE_UNIQUE_ID(scopeGuard) Y_DECLARE_UNUSED = ::NPrivate::TMakeGuardHelper{} | [__VA_ARGS__]() mutable -> void

// \brief `Y_DEFER { body };`
//
// Same as `Y_SCOPE_EXIT` but doesn't require user to provide capture-list explicitly (it
// implicitly uses `[&]` capture). Have same requirements for `body`.
//
// Inspired by `defer` statement in languages like Swift and Go.
//
// \code
// auto item = s.pop();
// bool ok = false;
// Y_DEFER { if (!ok) { s.push(std::move(item)); } };
// ... try handle `item` ...
// ok = true;
// \endcode
#define Y_DEFER Y_SCOPE_EXIT(&)
