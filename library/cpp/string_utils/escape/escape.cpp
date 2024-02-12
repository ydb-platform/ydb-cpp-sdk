#include "escape.h"

#include <util/system/defaults.h>
#include <util/system/yassert.h>

/// @todo: escape trigraphs (eg "??/" is "\")

/* REFEREBCES FOR ESCAPE SEQUENCE INTERPRETATION:
 *   C99 p. 6.4.3   Universal character names.
 *   C99 p. 6.4.4.4 Character constants.
 *
 * <simple-escape-sequence> ::= {
 *      \' , \" , \? , \\ ,
 *      \a , \b , \f , \n , \r , \t , \v
 * }
 *
 * <octal-escape-sequence>       ::= \  <octal-digit> {1, 3}
 * <hexadecimal-escape-sequence> ::= \x <hexadecimal-digit> +
 * <universal-character-name>    ::= \u <hexadecimal-digit> {4}
 *                                || \U <hexadecimal-digit> {8}
 *
 * NOTE (6.4.4.4.7):
 * Each octal or hexadecimal escape sequence is the longest sequence of characters that can
 * constitute the escape sequence.
 *
 * THEREFORE:
 *  - Octal escape sequence spans until rightmost non-octal-digit character.
 *  - Octal escape sequence always terminates after three octal digits.
 *  - Hexadecimal escape sequence spans until rightmost non-hexadecimal-digit character.
 *  - Universal character name consists of exactly 4 or 8 hexadecimal digit.
 *
 * by kerzum@
 * It is also required to escape trigraphs that are enabled in compilers by default and
 * are also processed inside string literals
 *      The nine trigraphs and their replacements are
 *
 *      Trigraph:       ??(  ??)  ??<  ??>  ??=  ??/  ??'  ??!  ??-
 *      Replacement:      [    ]    {    }    #    \    ^    |    ~
 *
 */
namespace {
    template <typename TChar>
    static inline char HexDigit(TChar value) {
        Y_ASSERT(value < 16);
        if (value < 10) {
            return '0' + value;
        } else {
            return 'A' + value - 10;
        }
    }

    template <typename TChar>
    static inline char OctDigit(TChar value) {
        Y_ASSERT(value < 8);
        return '0' + value;
    }

    template <typename TChar>
    static inline bool IsPrintable(TChar c) {
        return c >= 32 && c <= 126;
    }

    template <typename TChar>
    static inline bool IsHexDigit(TChar c) {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    }

    template <typename TChar>
    static inline bool IsOctDigit(TChar c) {
        return c >= '0' && c <= '7';
    }

    template <typename TChar>
    struct TEscapeUtil;

    template <>
    struct TEscapeUtil<char> {
        static const size_t ESCAPE_C_BUFFER_SIZE = 4;

        template <typename TNextChar, typename TBufferChar>
        static inline size_t EscapeC(unsigned char c, TNextChar next, TBufferChar r[ESCAPE_C_BUFFER_SIZE]) {
            // (1) Printable characters go as-is, except backslash and double quote.
            // (2) Characters \r, \n, \t and \0 ... \7 replaced by their simple escape characters (if possible).
            // (3) Otherwise, character is encoded using hexadecimal escape sequence (if possible), or octal.
            if (c == '\"') {
                r[0] = '\\';
                r[1] = '\"';
                return 2;
            } else if (c == '\\') {
                r[0] = '\\';
                r[1] = '\\';
                return 2;
            } else if (IsPrintable(c) && (!(c == '?' && next == '?'))) {
                r[0] = c;
                return 1;
            } else if (c == '\r') {
                r[0] = '\\';
                r[1] = 'r';
                return 2;
            } else if (c == '\n') {
                r[0] = '\\';
                r[1] = 'n';
                return 2;
            } else if (c == '\t') {
                r[0] = '\\';
                r[1] = 't';
                return 2;
            } else if (c < 8 && !IsOctDigit(next)) {
                r[0] = '\\';
                r[1] = OctDigit(c);
                return 2;
            } else if (!IsHexDigit(next)) {
                r[0] = '\\';
                r[1] = 'x';
                r[2] = HexDigit((c & 0xF0) >> 4);
                r[3] = HexDigit((c & 0x0F) >> 0);
                return 4;
            } else {
                r[0] = '\\';
                r[1] = OctDigit((c & 0700) >> 6);
                r[2] = OctDigit((c & 0070) >> 3);
                r[3] = OctDigit((c & 0007) >> 0);
                return 4;
            }
        }
    };

    template <>
    struct TEscapeUtil<wchar16> {
        static const size_t ESCAPE_C_BUFFER_SIZE = 6;

        template <typename TNextChar, typename TBufferChar>
        static inline size_t EscapeC(wchar16 c, TNextChar next, TBufferChar r[ESCAPE_C_BUFFER_SIZE]) {
            if (c < 0x100) {
                return TEscapeUtil<char>::EscapeC(char(c), next, r);
            } else {
                r[0] = '\\';
                r[1] = 'u';
                r[2] = HexDigit((c & 0xF000) >> 12);
                r[3] = HexDigit((c & 0x0F00) >> 8);
                r[4] = HexDigit((c & 0x00F0) >> 4);
                r[5] = HexDigit((c & 0x000F) >> 0);
                return 6;
            }
        }
    };
}

template <class TChar>
std::basic_string<TChar>& EscapeCImpl(const TChar* str, size_t len, std::basic_string<TChar>& r) {
    using TEscapeUtil = ::TEscapeUtil<TChar>;

    TChar buffer[TEscapeUtil::ESCAPE_C_BUFFER_SIZE];

    size_t i, j;
    for (i = 0, j = 0; i < len; ++i) {
        size_t rlen = TEscapeUtil::EscapeC(str[i], (i + 1 < len ? str[i + 1] : 0), buffer);

        if (rlen > 1) {
            r.append(str + j, i - j);
            j = i + 1;
            r.append(buffer, rlen);
        }
    }

    if (j > 0) {
        r.append(str + j, len - j);
    } else {
        r.append(str, len);
    }

    return r;
}

template std::string& EscapeCImpl<std::string::value_type>(const std::string::value_type* str, size_t len, std::string& r);
template std::u16string& EscapeCImpl<std::u16string::value_type>(const std::u16string::value_type* str, size_t len, std::u16string& r);

std::string& EscapeC(const std::string_view str, std::string& s) {
    return EscapeC(str.data(), str.size(), s);
}

std::u16string& EscapeC(const std::u16string_view str, std::u16string& w) {
    return EscapeC(str.data(), str.size(), w);
}

std::string EscapeC(const std::string& str) {
    return EscapeC(str.data(), str.size());
}

std::u16string EscapeC(const std::u16string& str) {
    return EscapeC(str.data(), str.size());
}

