#pragma once

#include <ydb-cpp-sdk/library/yson/public.h>
#include <ydb-cpp-sdk/library/yson/token.h>

#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <memory>

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    class TStatelessLexer {
    public:
        TStatelessLexer();

        ~TStatelessLexer();

        size_t GetToken(const std::string_view& data, TToken* token);

    private:
        class TImpl;
        std::unique_ptr<TImpl> Impl;
    };

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
