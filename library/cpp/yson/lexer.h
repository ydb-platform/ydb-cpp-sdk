#pragma once

#include "public.h"
#include "token.h"



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
