#pragma once


Y_PURE_FUNCTION bool IsSpace(const char* s, size_t len) noexcept;

/// Checks if a string is a set of only space symbols.
Y_PURE_FUNCTION static inline bool IsSpace(const std::string_view s) noexcept {
    return IsSpace(s.data(), s.size());
}

/// Returns "true" if the given string is an arabic number ([0-9]+)
Y_PURE_FUNCTION bool IsNumber(const std::string_view s) noexcept;

Y_PURE_FUNCTION bool IsNumber(const std::u16string_view s) noexcept;

/// Returns "true" if the given string is a hex number ([0-9a-fA-F]+)
Y_PURE_FUNCTION bool IsHexNumber(const std::string_view s) noexcept;

Y_PURE_FUNCTION bool IsHexNumber(const std::u16string_view s) noexcept;

/* Tests if the given string is case insensitive equal to one of:
 * - "true",
 * - "t",
 * - "yes",
 * - "y",
 * - "on",
 * - "1",
 * - "da".
 */
Y_PURE_FUNCTION bool IsTrue(const std::string_view value) noexcept;

/* Tests if the given string is case insensitive equal to one of:
 *  - "false",
 *  - "f",
 *  - "no",
 *  - "n",
 *  - "off",
 *  - "0",
 *  - "net".
 */
Y_PURE_FUNCTION bool IsFalse(const std::string_view value) noexcept;
