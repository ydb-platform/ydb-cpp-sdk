#include "type.h"
#include <ydb-cpp-sdk/util/string/ascii.h>

#include <array>

bool IsSpace(const char* s, size_t len) noexcept {
    if (len == 0) {
        return false;
    }
    for (const char* p = s; p < s + len; ++p) {
        if (!IsAsciiSpace(*p)) {
            return false;
        }
    }
    return true;
}

template <typename TStringType>
static bool IsNumberT(const TStringType& s) noexcept {
    if (s.empty()) {
        return false;
    }

    return std::all_of(s.begin(), s.end(), IsAsciiDigit<typename TStringType::value_type>);
}

bool IsNumber(const std::string_view s) noexcept {
    return IsNumberT(s);
}

bool IsNumber(const std::u16string_view s) noexcept {
    return IsNumberT(s);
}

template <typename TStringType>
static bool IsHexNumberT(const TStringType& s) noexcept {
    if (s.empty()) {
        return false;
    }

    return std::all_of(s.begin(), s.end(), IsAsciiHex<typename TStringType::value_type>);
}

bool IsHexNumber(const std::string_view s) noexcept {
    return IsHexNumberT(s);
}

bool IsHexNumber(const std::u16string_view s) noexcept {
    return IsHexNumberT(s);
}

namespace {
    template <size_t N>
    bool IsCaseInsensitiveAnyOf(std::string_view str, const std::array<std::string_view, N>& options) {
        for (auto option : options) {
            if (str.size() == option.size() && ::strnicmp(str.data(), option.data(), str.size()) == 0) {
                return true;
            }
        }
        return false;
    }
} //anonymous namespace

bool IsTrue(const std::string_view v) noexcept {
    static constexpr std::array<std::string_view, 7> trueOptions{
        "true",
        "t",
        "yes",
        "y",
        "on",
        "1",
        "da"};
    return IsCaseInsensitiveAnyOf(v, trueOptions);
}

bool IsFalse(const std::string_view v) noexcept {
    static constexpr std::array<std::string_view, 7> falseOptions{
        "false",
        "f",
        "no",
        "n",
        "off",
        "0",
        "net"};
    return IsCaseInsensitiveAnyOf(v, falseOptions);
}
