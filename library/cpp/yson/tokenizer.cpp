#include "tokenizer.h"

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    TTokenizer::TTokenizer(const std::string_view& input)
        : Input(input)
        , Parsed(0)
    {
    }

    bool TTokenizer::ParseNext() {
        Input = Input.Tail(Parsed);
        Token.Reset();
        Parsed = Lexer.GetToken(Input, &Token);
        return !CurrentToken().IsEmpty();
    }

    const TToken& TTokenizer::CurrentToken() const {
        return Token;
    }

    ETokenType TTokenizer::GetCurrentType() const {
        return CurrentToken().GetType();
    }

    std::string_view TTokenizer::GetCurrentSuffix() const {
        return Input.Tail(Parsed);
    }

    const std::string_view& TTokenizer::CurrentInput() const {
        return Input;
    }

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
