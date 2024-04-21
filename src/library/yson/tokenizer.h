#pragma once

#include <ydb-cpp-sdk/library/yson/public.h>
#include "lexer.h"

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    class TTokenizer {
    public:
        explicit TTokenizer(const std::string_view& input);

        bool ParseNext();
        const TToken& CurrentToken() const;
        ETokenType GetCurrentType() const;
        std::string_view GetCurrentSuffix() const;
        const std::string_view& CurrentInput() const;

    private:
        std::string_view Input;
        TToken Token;
        TStatelessLexer Lexer;
        size_t Parsed;
    };

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
