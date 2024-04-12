#pragma once

#include "codepage.h"
#include "iconv.h"

#include <util/charset/wide.h>
#include <string>

//! converts text from unicode to yandex codepage
//! @attention destination buffer must be long enough to fit all characters of the text
//! @note @c dest buffer must fit at least @c len number of characters
template <typename TCharType>
inline size_t WideToChar(const TCharType* text, size_t len, char* dest, ECharset enc) {
    Y_ASSERT(SingleByteCodepage(enc));

    const char* start = dest;

    const Encoder* const encoder = &EncoderByCharset(enc);
    const TCharType* const last = text + len;
    for (const TCharType* cur = text; cur != last; ++dest) {
        *dest = encoder->Tr(ReadSymbolAndAdvance(cur, last));
    }

    return dest - start;
}

//! converts text to unicode using a codepage object
//! @attention destination buffer must be long enough to fit all characters of the text
//! @note @c dest buffer must fit at least @c len number of characters;
//!       if you need convert zero terminated string you should determine length of the
//!       string using the @c strlen function and pass as the @c len parameter;
//!       it does not make sense to create an additional version of this function because
//!       it will call to @c strlen anyway in order to allocate destination buffer
template <typename TCharType>
inline void CharToWide(const char* text, size_t len, TCharType* dest, const CodePage& cp) {
    const unsigned char* cur = reinterpret_cast<const unsigned char*>(text);
    const unsigned char* const last = cur + len;
    for (; cur != last; ++cur, ++dest) {
        *dest = static_cast<TCharType>(cp.unicode[*cur]); // static_cast is safe as no 1char codepage contains non-BMP symbols
    }
}

namespace NDetail {
    namespace NBaseOps {
        // Template interface base recoding drivers, do not perform any memory management,
        // do not care about buffer size, so supplied @dst
        // should have enough room for the result (with proper reserve for the worst case)

        // Depending on template params, perform conversion of single-byte/multi-byte/utf8 string to/from wide string.

        template <typename TCharType>
        inline std::basic_string_view<TCharType> RecodeSingleByteChar(const std::string_view src, TCharType* dst, const CodePage& cp) {
            Y_ASSERT(cp.SingleByteCodepage());
            ::CharToWide(src.data(), src.size(), dst, cp);
            return std::basic_string_view<TCharType>(dst, src.size());
        }

        template <typename TCharType>
        inline std::string_view RecodeSingleByteChar(const std::basic_string_view<TCharType> src, char* dst, const CodePage& cp) {
            Y_ASSERT(cp.SingleByteCodepage());
            ::WideToChar(src.data(), src.size(), dst, cp.CPEnum);
            return std::string_view(dst, src.size());
        }

        template <typename TCharType>
        inline std::basic_string_view<TCharType> RecodeMultiByteChar(const std::string_view src, TCharType* dst, ECharset encoding) {
            Y_ASSERT(!NCodepagePrivate::NativeCodepage(encoding));
            size_t read = 0;
            size_t written = 0;
            ::NICONVPrivate::RecodeToUnicode(encoding, src.data(), dst, src.size(), src.size(), read, written);
            return std::basic_string_view<TCharType>(dst, written);
        }

        template <typename TCharType>
        inline std::string_view RecodeMultiByteChar(const std::basic_string_view<TCharType> src, char* dst, ECharset encoding) {
            Y_ASSERT(!NCodepagePrivate::NativeCodepage(encoding));
            size_t read = 0;
            size_t written = 0;
            ::NICONVPrivate::RecodeFromUnicode(encoding, src.data(), dst, src.size(), src.size() * 3, read, written);
            return std::string_view(dst, written);
        }

        template <typename TCharType>
        inline std::basic_string_view<TCharType> RecodeUtf8(const std::string_view src, TCharType* dst) {
            size_t len = 0;
            if (!::UTF8ToWide(src.data(), src.size(), dst, len))
                ythrow yexception() << "Invalid UTF8: \"" << src.substr(0, 50) << (src.size() > 50 ? "...\"" : "\"");
            return std::basic_string_view<TCharType>(dst, len);
        }

        template <typename TCharType>
        inline std::string_view RecodeUtf8(const std::basic_string_view<TCharType> src, char* dst) {
            size_t len = 0;
            ::WideToUTF8(src.data(), src.size(), dst, len);
            return std::string_view(dst, len);
        }

        // Select one of re-coding methods from above, based on provided @encoding

        template <typename TCharFrom, typename TCharTo>
        std::basic_string_view<TCharTo> Recode(const std::basic_string_view<TCharFrom> src, TCharTo* dst, ECharset encoding) {
            if (encoding == CODES_UTF8)
                return RecodeUtf8(src, dst);
            else if (SingleByteCodepage(encoding))
                return RecodeSingleByteChar(src, dst, *CodePageByCharset(encoding));
            else
                return RecodeMultiByteChar(src, dst, encoding);
        }

    }

    template <typename TCharFrom>
    struct TRecodeTraits;

    template <>
    struct TRecodeTraits<char> {
        using TCharTo = wchar16;
        using TStringViewTo = std::u16string_view;
        using TStringTo = std::u16string;
        enum { ReserveSize = 4 }; // How many TCharFrom characters we should reserve for one TCharTo character in worst case
                                  // Here an unicode character can be converted up to 4 bytes of UTF8
    };

    template <>
    struct TRecodeTraits<wchar16> {
        using TCharTo = char;
        using TStringViewTo = std::string_view;
        using TStringTo = std::string;
        enum { ReserveSize = 2 }; // possible surrogate pairs ?
    };

    // Operations with destination buffer where recoded string will be written
    template <typename TResult>
    struct TRecodeResultOps {
        // default implementation will work with std::string and std::u16string - 99% of usage
        using TResultChar = typename TResult::value_type;

        static inline size_t Size(const TResult& dst) {
            return dst.size();
        }

        static inline TResultChar* Reserve(TResult& dst, size_t len) {
            dst.resize(len);
            return dst.data();
        }

        static inline void Truncate(TResult& dst, size_t len) {
            dst.resize(len);
        }
    };

    // Main template interface for recoding in both directions

    template <typename TCharFrom, typename TResult>
    typename TRecodeTraits<TCharFrom>::TStringViewTo Recode(const std::basic_string_view<TCharFrom> src, TResult& dst, ECharset encoding) {
        using TCharTo = typename TRecodeTraits<TCharFrom>::TCharTo;
        // make enough room for re-coded string
        TCharTo* dstbuf = TRecodeResultOps<TResult>::Reserve(dst, src.size() * TRecodeTraits<TCharTo>::ReserveSize);
        // do re-coding
        std::basic_string_view<TCharTo> res = NBaseOps::Recode(src, dstbuf, encoding);
        // truncate result back to proper size
        TRecodeResultOps<TResult>::Truncate(dst, res.size());
        return res;
    }

    // appending version of Recode()
    template <typename TCharFrom, typename TResult>
    typename TRecodeTraits<TCharFrom>::TStringViewTo RecodeAppend(const std::basic_string_view<TCharFrom> src, TResult& dst, ECharset encoding) {
        using TCharTo = typename TRecodeTraits<TCharFrom>::TCharTo;
        size_t dstOrigSize = TRecodeResultOps<TResult>::Size(dst);
        TCharTo* dstbuf = TRecodeResultOps<TResult>::Reserve(dst, dstOrigSize + src.size() * TRecodeTraits<TCharTo>::ReserveSize);
        std::basic_string_view<TCharTo> appended = NBaseOps::Recode(src, dstbuf + dstOrigSize, encoding);
        size_t dstFinalSize = dstOrigSize + appended.size();
        TRecodeResultOps<TResult>::Truncate(dst, dstFinalSize);
        return std::basic_string_view<TCharTo>(dstbuf, dstFinalSize);
    }

    // special implementation for robust utf8 functions
    template <typename TResult>
    std::u16string_view RecodeUTF8Robust(const std::string_view src, TResult& dst) {
        // make enough room for re-coded string
        wchar16* dstbuf = TRecodeResultOps<TResult>::Reserve(dst, src.size() * TRecodeTraits<wchar16>::ReserveSize);

        // do re-coding
        size_t written = 0;
        UTF8ToWide<true>(src.data(), src.size(), dstbuf, written);

        // truncate result back to proper size
        TRecodeResultOps<TResult>::Truncate(dst, written);
        return std::u16string_view(dstbuf, written);
    }

    template <typename TCharFrom>
    inline typename TRecodeTraits<TCharFrom>::TStringTo Recode(const std::basic_string_view<TCharFrom> src, ECharset encoding) {
        typename TRecodeTraits<TCharFrom>::TStringTo res;
        Recode<TCharFrom>(src, res, encoding);
        return res;
    }
}

// Write result into @dst. Return string-buffer pointing to re-coded content of @dst.

template <bool robust>
inline std::u16string_view CharToWide(const std::string_view src, std::u16string& dst, ECharset encoding) {
    if (robust && CODES_UTF8 == encoding)
        return ::NDetail::RecodeUTF8Robust(src, dst);
    return ::NDetail::Recode<char>(src, dst, encoding);
}

inline std::u16string_view CharToWide(const std::string_view src, std::u16string& dst, ECharset encoding) {
    return ::NDetail::Recode<char>(src, dst, encoding);
}

inline std::string_view WideToChar(const std::u16string_view src, std::string& dst, ECharset encoding) {
    return ::NDetail::Recode<wchar16>(src, dst, encoding);
}

//! calls either to @c WideToUTF8 or @c WideToChar depending on the encoding type
inline std::string WideToChar(const wchar16* text, size_t len, ECharset enc) {
    if (NCodepagePrivate::NativeCodepage(enc)) {
        if (enc == CODES_UTF8)
            return WideToUTF8(text, len);

        std::string s(len, '\0');
        s.erase(WideToChar(text, len, s.data(), enc));

        return s;
    }

    std::string s(len * 3, '\0');

    size_t read = 0;
    size_t written = 0;
    NICONVPrivate::RecodeFromUnicode(enc, text, s.data(), len, s.size(), read, written);
    s.erase(written);

    return s;
}

inline std::u16string CharToWide(const char* text, size_t len, const CodePage& cp) {
    std::u16string w(len, wchar16());
    CharToWide(text, len, w.data(), cp);
    return w;
}

//! calls either to @c UTF8ToWide or @c CharToWide depending on the encoding type
template <bool robust>
inline std::u16string CharToWide(const char* text, size_t len, ECharset enc) {
    if (NCodepagePrivate::NativeCodepage(enc)) {
        if (enc == CODES_UTF8)
            return UTF8ToWide<robust>(text, len);

        return CharToWide(text, len, *CodePageByCharset(enc));
    }

    std::u16string w(len * 2, wchar16());

    size_t read = 0;
    size_t written = 0;
    NICONVPrivate::RecodeToUnicode(enc, text, w.data(), len, len, read, written);
    w.erase(written);

    return w;
}

//! converts text from UTF8 to unicode, if conversion fails it uses codepage to convert the text
//! @param text     text to be converted
//! @param len      length of the text in characters
//! @param cp       a codepage that is used in case of failed conversion from UTF8
inline std::u16string UTF8ToWide(const char* text, size_t len, const CodePage& cp) {
    std::u16string w(len, wchar16());
    size_t written = 0;
    if (UTF8ToWide(text, len, w.data(), written))
        w.erase(written);
    else
        CharToWide(text, len, w.data(), cp);
    return w;
}

inline std::string WideToChar(const std::u16string_view w, ECharset enc) {
    return WideToChar(w.data(), w.size(), enc);
}

inline std::u16string CharToWide(const std::string_view s, ECharset enc) {
    return CharToWide<false>(s.data(), s.size(), enc);
}

template <bool robust>
inline std::u16string CharToWide(const std::string_view s, ECharset enc) {
    return CharToWide<robust>(s.data(), s.size(), enc);
}

inline std::u16string CharToWide(const std::string_view s, const CodePage& cp) {
    return CharToWide(s.data(), s.size(), cp);
}

// true if @text can be fully encoded to specified @encoding,
// with possibility to recover exact original text after decoding
bool CanBeEncoded(std::u16string_view text, ECharset encoding);
