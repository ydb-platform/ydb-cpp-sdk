#include "wide.h"

#include <src/util/generic/mem_copy.h>
#include <ydb-cpp-sdk/util/string/strip.h>

namespace {
    //! the constants are not zero-terminated
    constexpr wchar16 LT[] = {'&', 'l', 't', ';'};
    constexpr wchar16 GT[] = {'&', 'g', 't', ';'};
    constexpr wchar16 AMP[] = {'&', 'a', 'm', 'p', ';'};
    constexpr wchar16 BR[] = {'<', 'B', 'R', '>'};
    constexpr wchar16 QUOT[] = {'&', 'q', 'u', 'o', 't', ';'};

    template <bool insertBr>
    inline size_t EscapedLen(wchar16 c) {
        switch (c) {
            case '<':
                return Y_ARRAY_SIZE(LT);
            case '>':
                return Y_ARRAY_SIZE(GT);
            case '&':
                return Y_ARRAY_SIZE(AMP);
            case '\"':
                return Y_ARRAY_SIZE(QUOT);
            default:
                if (insertBr && (c == '\r' || c == '\n')) {
                    return Y_ARRAY_SIZE(BR);
                } else {
                    return 1;
                }
        }
    }
}

void Collapse(std::u16string& w) {
    CollapseImpl(w, w, 0, IsWhitespace);
}

size_t Collapse(wchar16* s, size_t n) {
    return CollapseImpl(s, n, IsWhitespace);
}

std::u16string_view StripLeft(const std::u16string_view text) noexcept {
    const auto* p = text.data();
    const auto* const pe = text.data() + text.size();

    for (; p != pe && IsWhitespace(*p); ++p) {
    }

    return {p, pe};
}

void StripLeft(std::u16string& text) {
    const auto stripped = StripLeft(std::u16string_view(text));
    if (stripped.size() == text.size()) {
        return;
    }

    text = stripped;
}

std::u16string_view StripRight(const std::u16string_view text) noexcept {
    if (text.empty()) {
        return {};
    }

    const auto* const pe = text.data() - 1;
    const auto* p = text.data() + text.size() - 1;

    for (; p != pe && IsWhitespace(*p); --p) {
    }

    return {pe + 1, p + 1};
}

void StripRight(std::u16string& text) {
    const auto stripped = StripRight(std::u16string_view(text));
    if (stripped.size() == text.size()) {
        return;
    }

    text.resize(stripped.size());
}

std::u16string_view Strip(const std::u16string_view text) noexcept {
    return StripRight(StripLeft(text));
}

void Strip(std::u16string& text) {
    StripLeft(text);
    StripRight(text);
}

template <typename T>
static bool IsReductionOnSymbolsTrue(const std::u16string_view text, T&& f) {
    const auto* p = text.data();
    const auto* const pe = text.data() + text.length();
    while (p != pe) {
        const auto symbol = ReadSymbolAndAdvance(p, pe);
        if (!f(symbol)) {
            return false;
        }
    }

    return true;
}

bool IsLowerWord(const std::u16string_view text) noexcept {
    return IsReductionOnSymbolsTrue(text, [](const wchar32 s) { return IsLower(s); });
}

bool IsUpperWord(const std::u16string_view text) noexcept {
    return IsReductionOnSymbolsTrue(text, [](const wchar32 s) { return IsUpper(s); });
}

bool IsLower(const std::u16string_view text) noexcept {
    return IsReductionOnSymbolsTrue(text, [](const wchar32 s) {
        if (IsAlpha(s)) {
            return IsLower(s);
        }
        return true;
    });
}

bool IsUpper(const std::u16string_view text) noexcept {
    return IsReductionOnSymbolsTrue(text, [](const wchar32 s) {
        if (IsAlpha(s)) {
            return IsUpper(s);
        }
        return true;
    });
}

bool IsTitleWord(const std::u16string_view text) noexcept {
    if (text.empty()) {
        return false;
    }

    const auto* p = text.data();
    const auto* pe = text.data() + text.size();

    const auto firstSymbol = ReadSymbolAndAdvance(p, pe);
    if (firstSymbol != ToTitle(firstSymbol)) {
        return false;
    }

    return IsLowerWord({p, pe});
}

template <bool stopOnFirstModification, typename TCharType, typename F>
static bool ModifySequence(TCharType*& p, const TCharType* const pe, F&& f) {
    while (p != pe) {
        const auto symbol = ReadSymbol(p, pe);
        const auto modified = f(symbol);
        if (symbol != modified) {
            if constexpr (stopOnFirstModification) {
                return true;
            }

            WriteSymbol(modified, p); // also moves `p` forward
        } else {
            p = SkipSymbol(p, pe);
        }
    }

    return false;
}

template <bool stopOnFirstModification, typename TCharType, typename F>
static bool ModifySequence(const TCharType*& p, const TCharType* const pe, TCharType*& out, F&& f) {
    using TSymbol = decltype(stopOnFirstModification ? ReadSymbol(p, pe) : ReadSymbolAndAdvance(p, pe));
    
    while (p != pe) {
        TSymbol symbol;
        if constexpr (stopOnFirstModification) {
            symbol = ReadSymbol(p, pe);
        } else {
            symbol = ReadSymbolAndAdvance(p, pe);
        }
        const auto modified = f(symbol);

        if constexpr (stopOnFirstModification) {
            if (symbol != modified) {
                return true;
            }

            p = SkipSymbol(p, pe);
        }

        WriteSymbol(modified, out);
    }

    return false;
}

template <class TStringType, typename F>
static bool ModifyStringSymbolwise(TStringType& text, size_t pos, size_t count, F&& f) {
    // TODO(yazevnul): this is done for consistency with old `TUtf16String::to_lower` and friends
    // at r2914050, maybe worth replacing them with asserts. Also see the same code in `ToTitle`.
    pos = std::min(pos, text.size());
    count = std::min(count, text.size() - pos);

    auto* p = text.data() + pos;
    const auto* pe = text.data() + pos + count;

    if (ModifySequence<true>(p, pe, f)) {
        ModifySequence<false>(p, pe, f);
        return true;
    }

    return false;
}

bool ToLower(std::u16string& text, size_t pos, size_t count) {
    const auto f = [](const wchar32 s) { return ToLower(s); };
    return ModifyStringSymbolwise(text, pos, count, f);
}

bool ToUpper(std::u16string& text, size_t pos, size_t count) {
    const auto f = [](const wchar32 s) { return ToUpper(s); };
    return ModifyStringSymbolwise(text, pos, count, f);
}

bool ToLower(std::u32string& text, size_t pos, size_t count) {
    const auto f = [](const wchar32 s) { return ToLower(s); };
    return ModifyStringSymbolwise(text, pos, count, f);
}

bool ToUpper(std::u32string& text, size_t pos, size_t count) {
    const auto f = [](const wchar32 s) { return ToUpper(s); };
    return ModifyStringSymbolwise(text, pos, count, f);
}

bool ToTitle(std::u16string& text, size_t pos, size_t count) {
    if (text.empty()) {
        return false;
    }

    pos = std::min(pos, text.size());
    count = std::min(text.size(), text.size() - pos);

    const auto toLower = [](const wchar32 s) { return ToLower(s); };

    auto* p = const_cast<wchar16*>(text.data() + pos);
    const auto* pe = text.data() + pos + count;

    const auto firstSymbol = ReadSymbol(p, pe);
    if (firstSymbol == ToTitle(firstSymbol)) {
        p = SkipSymbol(p, pe);
        if (ModifySequence<true>(p, pe, toLower)) {
            ModifySequence<false>(p, pe, toLower);
            return true;
        }
    } else {
        WriteSymbol(ToTitle(ReadSymbol(p, pe)), p); // also moves `p` forward
        ModifySequence<false>(p, pe, toLower);
        return true;
    }

    return false;
}

bool ToTitle(std::u32string& text, size_t pos, size_t count) {
    if (text.empty()) {
        return false;
    }

    pos = std::min(pos, text.size());
    count = std::min(count, text.size() - pos);

    const auto toLower = [](const wchar32 s) { return ToLower(s); };

    auto* p = const_cast<wchar32*>(text.data() + pos);
    const auto* pe = text.data() + pos + count;

    const auto firstSymbol = *p;
    if (firstSymbol == ToTitle(firstSymbol)) {
        p += 1;
        if (ModifySequence<true>(p, pe, toLower)) {
            ModifySequence<false>(p, pe, toLower);
            return true;
        }
    } else {
        WriteSymbol(ToTitle(ReadSymbol(p, pe)), p); // also moves `p` forward
        ModifySequence<false>(p, pe, toLower);
        return true;
    }

    return false;
}

std::u16string ToLowerRet(std::u16string text, size_t pos, size_t count) {
    ToLower(text, pos, count);
    return text;
}

std::u16string ToUpperRet(std::u16string text, size_t pos, size_t count) {
    ToUpper(text, pos, count);
    return text;
}

std::u16string ToTitleRet(std::u16string text, size_t pos, size_t count) {
    ToTitle(text, pos, count);
    return text;
}

std::u32string ToLowerRet(std::u32string text, size_t pos, size_t count) {
    ToLower(text, pos, count);
    return text;
}

std::u32string ToUpperRet(std::u32string text, size_t pos, size_t count) {
    ToUpper(text, pos, count);
    return text;
}

std::u32string ToTitleRet(std::u32string text, size_t pos, size_t count) {
    ToTitle(text, pos, count);
    return text;
}

bool ToLower(const wchar16* text, size_t length, wchar16* out) noexcept {
    // TODO(yazevnul): get rid of `text == out` case (it is probably used only in lemmer) and then
    // we can declare text and out as `__restrict__`
    Y_ASSERT(text == out || !(out >= text && out < text + length));
    const auto f = [](const wchar32 s) { return ToLower(s); };
    const auto* p = text;
    const auto* const pe = text + length;
    if (ModifySequence<true>(p, pe, out, f)) {
        ModifySequence<false>(p, pe, out, f);
        return true;
    }
    return false;
}

bool ToUpper(const wchar16* text, size_t length, wchar16* out) noexcept {
    Y_ASSERT(text == out || !(out >= text && out < text + length));
    const auto f = [](const wchar32 s) { return ToUpper(s); };
    const auto* p = text;
    const auto* const pe = text + length;
    if (ModifySequence<true>(p, pe, out, f)) {
        ModifySequence<false>(p, pe, out, f);
        return true;
    }
    return false;
}

bool ToTitle(const wchar16* text, size_t length, wchar16* out) noexcept {
    if (!length) {
        return false;
    }

    Y_ASSERT(text == out || !(out >= text && out < text + length));

    const auto* const textEnd = text + length;
    const auto firstSymbol = ReadSymbolAndAdvance(text, textEnd);
    const auto firstSymbolTitle = ToTitle(firstSymbol);

    WriteSymbol(firstSymbolTitle, out);

    return ToLower(text, textEnd - text, out) || firstSymbol != firstSymbolTitle;
}

bool ToLower(wchar16* text, size_t length) noexcept {
    const auto f = [](const wchar32 s) { return ToLower(s); };
    const auto* const textEnd = text + length;
    if (ModifySequence<true>(text, textEnd, f)) {
        ModifySequence<false>(text, textEnd, f);
        return true;
    }
    return false;
}

bool ToUpper(wchar16* text, size_t length) noexcept {
    const auto f = [](const wchar32 s) { return ToUpper(s); };
    const auto* const textEnd = text + length;
    if (ModifySequence<true>(text, textEnd, f)) {
        ModifySequence<false>(text, textEnd, f);
        return true;
    }
    return false;
}

bool ToTitle(wchar16* text, size_t length) noexcept {
    if (!length) {
        return false;
    }

    const auto* textEnd = text + length;
    const auto firstSymbol = ReadSymbol(text, textEnd);
    const auto firstSymbolTitle = ToTitle(firstSymbol);

    // avoid unnacessary writes to the memory
    if (firstSymbol != firstSymbolTitle) {
        WriteSymbol(firstSymbolTitle, text);
    } else {
        text = SkipSymbol(text, textEnd);
    }

    return ToLower(text, textEnd - text) || firstSymbol != firstSymbolTitle;
}

bool ToLower(const wchar32* text, size_t length, wchar32* out) noexcept {
    // TODO(yazevnul): get rid of `text == out` case (it is probably used only in lemmer) and then
    // we can declare text and out as `__restrict__`
    Y_ASSERT(text == out || !(out >= text && out < text + length));
    const auto f = [](const wchar32 s) { return ToLower(s); };
    const auto* p = text;
    const auto* const pe = text + length;
    if (ModifySequence<true>(p, pe, out, f)) {
        ModifySequence<false>(p, pe, out, f);
        return true;
    }
    return false;
}

bool ToUpper(const wchar32* text, size_t length, wchar32* out) noexcept {
    Y_ASSERT(text == out || !(out >= text && out < text + length));
    const auto f = [](const wchar32 s) { return ToUpper(s); };
    const auto* p = text;
    const auto* const pe = text + length;
    if (ModifySequence<true>(p, pe, out, f)) {
        ModifySequence<false>(p, pe, out, f);
        return true;
    }
    return false;
}

bool ToTitle(const wchar32* text, size_t length, wchar32* out) noexcept {
    if (!length) {
        return false;
    }

    Y_ASSERT(text == out || !(out >= text && out < text + length));

    const auto* const textEnd = text + length;
    const auto firstSymbol = ReadSymbolAndAdvance(text, textEnd);
    const auto firstSymbolTitle = ToTitle(firstSymbol);

    WriteSymbol(firstSymbolTitle, out);

    return ToLower(text, textEnd - text, out) || firstSymbol != firstSymbolTitle;
}

bool ToLower(wchar32* text, size_t length) noexcept {
    const auto f = [](const wchar32 s) { return ToLower(s); };
    const auto* const textEnd = text + length;
    if (ModifySequence<true>(text, textEnd, f)) {
        ModifySequence<false>(text, textEnd, f);
        return true;
    }
    return false;
}

bool ToUpper(wchar32* text, size_t length) noexcept {
    const auto f = [](const wchar32 s) { return ToUpper(s); };
    const auto* const textEnd = text + length;
    if (ModifySequence<true>(text, textEnd, f)) {
        ModifySequence<false>(text, textEnd, f);
        return true;
    }
    return false;
}

bool ToTitle(wchar32* text, size_t length) noexcept {
    if (!length) {
        return false;
    }

    const auto* textEnd = text + length;
    const auto firstSymbol = ReadSymbol(text, textEnd);
    const auto firstSymbolTitle = ToTitle(firstSymbol);

    // avoid unnacessary writes to the memory
    if (firstSymbol != firstSymbolTitle) {
        WriteSymbol(firstSymbolTitle, text);
    } else {
        text = SkipSymbol(text, textEnd);
    }

    return ToLower(text, textEnd - text) || firstSymbol != firstSymbolTitle;
}

template <typename F>
static std::u16string ToSmthRet(const std::u16string_view text, size_t pos, size_t count, F&& f) {
    pos = std::min(pos, text.size());
    count = std::min(count, text.size() - pos);

    std::u16string res(text.size(), '\0');
    auto* const resBegin = res.data();

    if (pos) {
        MemCopy(resBegin, text.data(), pos);
    }

    f(text.data() + pos, count, resBegin + pos);

    if (count - pos != text.size()) {
        MemCopy(resBegin + pos + count, text.data() + pos + count, text.size() - pos - count);
    }

    return res;
}

template <typename F>
static std::u32string ToSmthRet(const std::u32string_view text, size_t pos, size_t count, F&& f) {
    pos = std::min(pos, text.size());
    count = std::min(count, text.size() - pos);

    std::u32string res(text.size(), '\0');
    auto* const resBegin = res.data();

    if (pos) {
        MemCopy(resBegin, text.data(), pos);
    }

    f(text.data() + pos, count, resBegin + pos);

    if (count - pos != text.size()) {
        MemCopy(resBegin + pos + count, text.data() + pos + count, text.size() - pos - count);
    }

    return res;
}

std::u16string ToLowerRet(const std::u16string_view text, size_t pos, size_t count) {
    return ToSmthRet(text, pos, count, [](const wchar16* theText, size_t length, wchar16* out) {
        ToLower(theText, length, out);
    });
}

std::u16string ToUpperRet(const std::u16string_view text, size_t pos, size_t count) {
    return ToSmthRet(text, pos, count, [](const wchar16* theText, size_t length, wchar16* out) {
        ToUpper(theText, length, out);
    });
}

std::u16string ToTitleRet(const std::u16string_view text, size_t pos, size_t count) {
    return ToSmthRet(text, pos, count, [](const wchar16* theText, size_t length, wchar16* out) {
        ToTitle(theText, length, out);
    });
}

std::u32string ToLowerRet(const std::u32string_view text, size_t pos, size_t count) {
    return ToSmthRet(text, pos, count, [](const wchar32* theText, size_t length, wchar32* out) {
        ToLower(theText, length, out);
    });
}

std::u32string ToUpperRet(const std::u32string_view text, size_t pos, size_t count) {
    return ToSmthRet(text, pos, count, [](const wchar32* theText, size_t length, wchar32* out) {
        ToUpper(theText, length, out);
    });
}

std::u32string ToTitleRet(const std::u32string_view text, size_t pos, size_t count) {
    return ToSmthRet(text, pos, count, [](const wchar32* theText, size_t length, wchar32* out) {
        ToTitle(theText, length, out);
    });
}

template <bool insertBr>
void EscapeHtmlChars(std::u16string& str) {
    static const std::u16string lt(LT, Y_ARRAY_SIZE(LT));
    static const std::u16string gt(GT, Y_ARRAY_SIZE(GT));
    static const std::u16string amp(AMP, Y_ARRAY_SIZE(AMP));
    static const std::u16string br(BR, Y_ARRAY_SIZE(BR));
    static const std::u16string quot(QUOT, Y_ARRAY_SIZE(QUOT));

    size_t escapedLen = 0;

    const std::u16string& cs = str;

    for (size_t i = 0; i < cs.size(); ++i) {
        escapedLen += EscapedLen<insertBr>(cs[i]);
    }

    if (escapedLen == cs.size()) {
        return;
    }

    std::u16string res;
    res.reserve(escapedLen);

    size_t start = 0;

    for (size_t i = 0; i < cs.size(); ++i) {
        const std::u16string* ent = nullptr;
        switch (cs[i]) {
            case '<':
                ent = &lt;
                break;
            case '>':
                ent = &gt;
                break;
            case '&':
                ent = &amp;
                break;
            case '\"':
                ent = &quot;
                break;
            default:
                if (insertBr && (cs[i] == '\r' || cs[i] == '\n')) {
                    ent = &br;
                    break;
                } else {
                    continue;
                }
        }

        res.append(cs.begin() + start, cs.begin() + i);
        res.append(ent->begin(), ent->end());
        start = i + 1;
    }

    res.append(cs.begin() + start, cs.end());
    res.swap(str);
}

template void EscapeHtmlChars<false>(std::u16string& str);
template void EscapeHtmlChars<true>(std::u16string& str);
