#include "lexer.h"
#include "lexer_detail.h"
#include <ydb-cpp-sdk/library/yson/token.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    class TStatelessLexer::TImpl {
    private:
        THolder<TStatelessYsonLexerImplBase> Impl;

    public:
        TImpl(bool enableLinePositionInfo = false)
            : Impl(enableLinePositionInfo
                       ? static_cast<TStatelessYsonLexerImplBase*>(new TStatelesYsonLexerImpl<true>())
                       : static_cast<TStatelessYsonLexerImplBase*>(new TStatelesYsonLexerImpl<false>()))
                   {
        }

        size_t GetToken(const std::string_view& data, TToken* token) {
            return Impl->GetToken(data, token);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////

    TStatelessLexer::TStatelessLexer()
        : Impl(new TImpl())
    {
    }

    TStatelessLexer::~TStatelessLexer() {
    }

    size_t TStatelessLexer::GetToken(const std::string_view& data, TToken* token) {
        return Impl->GetToken(data, token);
    }

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
