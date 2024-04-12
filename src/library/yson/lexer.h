#pragma once

#include <ydb-cpp-sdk/library/yson/public.h>
#include <ydb-cpp-sdk/library/yson/token.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    class TStatelessLexer {
    public:
        TStatelessLexer();

        ~TStatelessLexer();

        size_t GetToken(const std::string_view& data, TToken* token);

    private:
        class TImpl;
        THolder<TImpl> Impl;
    };

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
