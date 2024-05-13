#include "unidata.h"
#include "utf8.h"

namespace {
    enum class ECaseConversion {
        ToUpper,
        ToLower,
    };

    wchar32 ConvertChar(ECaseConversion conversion, wchar32 ch) {
        switch (conversion) {
            case ECaseConversion::ToUpper:
                return ToUpper(ch);
            case ECaseConversion::ToLower:
                return ToLower(ch);
        }
        Y_ASSERT(false); // NOTREACHED
        return 0;
    }

    bool ConvertCaseUTF8Impl(ECaseConversion conversion, const char* beg, size_t n,
                             std::string& newString) {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(beg);
        const unsigned char* const end = p + n;

        // first loop searches for the first character, which is changed by ConvertChar
        // if there is no changed character, we don't need reallocation/copy
        wchar32 cNew = 0;
        size_t cLen = 0;
        while (p < end) {
            wchar32 c;
            if (RECODE_OK != SafeReadUTF8Char(c, cLen, p, end)) {
                ythrow yexception()
                    << "failed to decode UTF-8 string at pos "
                    << (reinterpret_cast<const char*>(p) - beg);
            }
            cNew = ConvertChar(conversion, c);

            if (cNew != c) {
                break;
            }
            p += cLen;
        }
        if (p == end) {
            return false;
        }

        // some character changed after ToLower. Write new string to newString.
        newString.resize(n);

        size_t written = reinterpret_cast<const char*>(p) - beg;
        char* writePtr = newString.data();
        memcpy(writePtr, beg, written);
        writePtr += written;
        size_t destSpace = n - written;

        // before each iteration (including the first one) variable 'cNew' contains unwritten symbol
        while (true) {
            size_t cNewLen;
            Y_ASSERT((writePtr - newString.data()) + destSpace == newString.size());
            if (RECODE_EOOUTPUT ==
                    SafeWriteUTF8Char(cNew, cNewLen,
                                      reinterpret_cast<unsigned char*>(writePtr), destSpace)) {
                destSpace += newString.size();
                newString.resize(newString.size() * 2);
                writePtr = newString.data() + (newString.size() - destSpace);
                continue;
            }
            destSpace -= cNewLen;
            writePtr += cNewLen;
            p += cLen;
            if (p == end) {
                newString.resize(newString.size() - destSpace);
                return true;
            }
            wchar32 c = 0;
            if (RECODE_OK != SafeReadUTF8Char(c, cLen, p, end)) {
                ythrow yexception()
                    << "failed to decode UTF-8 string at pos "
                    << (reinterpret_cast<const char*>(p) - beg);
            }
            cNew = ConvertChar(conversion, c);
        }
        Y_ASSERT(false);
        return false;
    }
} // namespace

extern const wchar32 BROKEN_RUNE = 0xFFFD;

static const char* SkipUTF8Chars(const char* begin, const char* end, size_t numChars) {
    const unsigned char* uEnd = reinterpret_cast<const unsigned char*>(end);
    while (begin != end && numChars > 0) {
        const unsigned char* uBegin = reinterpret_cast<const unsigned char*>(begin);
        size_t runeLen;
        if (GetUTF8CharLen(runeLen, uBegin, uEnd) != RECODE_OK) {
            ythrow yexception() << "invalid UTF-8 char";
        }
        begin += runeLen;
        Y_ASSERT(begin <= end);
        --numChars;
    }
    return begin;
}

std::string_view SubstrUTF8(const std::string_view str, size_t pos, size_t len) {
    const char* start = SkipUTF8Chars(str.begin(), str.end(), pos);
    const char* end = SkipUTF8Chars(start, str.end(), len);
    return std::string_view(start, end - start);
}

EUTF8Detect UTF8Detect(const char* s, size_t len) {
    const unsigned char* s0 = reinterpret_cast<const unsigned char*>(s);
    const unsigned char* send = s0 + len;
    wchar32 rune;
    size_t rune_len;
    EUTF8Detect res = ASCII;

    while (s0 < send) {
        RECODE_RESULT rr = SafeReadUTF8Char(rune, rune_len, s0, send);

        if (rr != RECODE_OK) {
            return NotUTF8;
        }

        if (rune_len > 1) {
            res = UTF8;
        }

        s0 += rune_len;
    }

    return res;
}

bool ToLowerUTF8Impl(const char* beg, size_t n, std::string& newString) {
    return ConvertCaseUTF8Impl(ECaseConversion::ToLower, beg, n, newString);
}

std::string ToLowerUTF8(std::string_view s) {
    std::string newString;
    bool changed = ToLowerUTF8Impl(s.data(), s.size(), newString);
    return changed ? newString : std::string(s.data(), s.size());
}

std::string ToLowerUTF8(const char* s) {
    return ToLowerUTF8(std::string_view(s));
}

bool ToUpperUTF8Impl(const char* beg, size_t n, std::string& newString) {
    return ConvertCaseUTF8Impl(ECaseConversion::ToUpper, beg, n, newString);
}

std::string ToUpperUTF8(const std::string& s) {
    std::string newString;
    bool changed = ToUpperUTF8Impl(s.data(), s.size(), newString);
    return changed ? newString : s;
}

std::string ToUpperUTF8(std::string_view s) {
    std::string newString;
    bool changed = ToUpperUTF8Impl(s.data(), s.size(), newString);
    return changed ? newString : std::string(s.data(), s.size());
}

std::string ToUpperUTF8(const char* s) {
    return ToUpperUTF8(std::string_view(s));
}
