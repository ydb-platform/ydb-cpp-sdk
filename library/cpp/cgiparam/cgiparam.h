#pragma once

#include <library/cpp/iterator/iterate_values.h>

#include <util/generic/iterator_range.h>
#include <util/generic/strbuf.h>
#include <util/generic/string.h>

#include <map>
#include <initializer_list>

class TCgiParameters: public std::multimap<std::string, std::string> {
public:
    TCgiParameters() = default;

    explicit TCgiParameters(const std::string_view cgiParamStr) {
        Scan(cgiParamStr);
    }

    TCgiParameters(std::initializer_list<std::pair<std::string, std::string>> il);

    void Flush() {
        erase(begin(), end());
    }

    size_t EraseAll(const std::string_view name);

    size_t NumOfValues(const std::string_view name) const noexcept {
        return count(static_cast<std::string>(name));
    }

    std::string operator()() const {
        return Print();
    }

    void Scan(const std::string_view cgiParStr, bool form = true);
    void ScanAdd(const std::string_view cgiParStr);
    void ScanAddUnescaped(const std::string_view cgiParStr);
    void ScanAddAllUnescaped(const std::string_view cgiParStr);
    void ScanAddAll(const std::string_view cgiParStr);

    /// Returns the string representation of all the stored parameters
    /**
     * @note The returned string has format <name1>=<value1>&<name2>=<value2>&...
     * @note Names and values in the returned string are CGI-escaped.
     */
    std::string Print() const;
    char* Print(char* res) const;

    Y_PURE_FUNCTION
    size_t PrintSize() const noexcept;

    /** The same as Print* except that RFC-3986 reserved characters are escaped.
     * @param safe - set of characters to be skipped in escaping
     */
    std::string QuotedPrint(const char* safe = "/") const;

    Y_PURE_FUNCTION
    auto Range(const std::string_view name) const noexcept {
        return IterateValues(MakeIteratorRange(equal_range(static_cast<std::string>(name))));
    }

    Y_PURE_FUNCTION
    const_iterator Find(const std::string_view name, size_t numOfValue = 0) const noexcept;

    Y_PURE_FUNCTION
    bool Has(const std::string_view name, const std::string_view value) const noexcept;

    Y_PURE_FUNCTION
    bool Has(const std::string_view name) const noexcept {
        const auto pair = equal_range(static_cast<std::string>(name));
        return pair.first != pair.second;
    }
    /// Returns value by name
    /**
     * @note The returned value is CGI-unescaped.
     */
    Y_PURE_FUNCTION
    const std::string& Get(const std::string_view name, size_t numOfValue = 0) const noexcept;

    void InsertEscaped(const std::string_view name, const std::string_view value);

#if !defined(__GLIBCXX__)
    template <typename TName, typename TValue>
    inline void InsertUnescaped(TName&& name, TValue&& value) {
        // std::string_view use as TName or TValue is C++17 actually.
        // There is no pair constructor available in C++14 when required type
        // is not implicitly constructible from given type.
        // But libc++ pair allows this with C++14.
        emplace(std::forward<TName>(name), std::forward<TValue>(value));
    }
#else
    template <typename TName, typename TValue>
    inline void InsertUnescaped(TName&& name, TValue&& value) {
        emplace(std::string(name), std::string(value));
    }
#endif

    // replace all values for a given key with new values
    template <typename TIter>
    void ReplaceUnescaped(const std::string_view key, TIter valuesBegin, const TIter valuesEnd);

    void ReplaceUnescaped(const std::string_view key, std::initializer_list<std::string_view> values) {
        ReplaceUnescaped(key, values.begin(), values.end());
    }

    void ReplaceUnescaped(const std::string_view key, const std::string_view value) {
        ReplaceUnescaped(key, {value});
    }

    // join multiple values into a single one using a separator
    // if val is a [possibly empty] non-NULL string, append it as well
    void JoinUnescaped(const std::string_view key, char sep, std::string_view val = std::string_view());

    bool Erase(const std::string_view name, size_t numOfValue = 0);
    bool Erase(const std::string_view name, const std::string_view val);
    bool ErasePattern(const std::string_view name, const std::string_view pat);

    inline const char* FormField(const std::string_view name, size_t numOfValue = 0) const {
        const_iterator it = Find(name, numOfValue);

        if (it == end()) {
            return nullptr;
        }

        return it->second.data();
    }

    inline std::string_view FormFieldBuf(const std::string_view name, size_t numOfValue = 0) const {
        const_iterator it = Find(name, numOfValue);

        if (it == end()) {
            return nullptr;
        }

        return it->second.data();
    }
};

template <typename TIter>
void TCgiParameters::ReplaceUnescaped(const std::string_view key, TIter valuesBegin, const TIter valuesEnd) {
    const auto oldRange = equal_range(static_cast<std::string>(key));
    auto current = oldRange.first;

    // reuse as many existing nodes as possible (probably none)
    for (; valuesBegin != valuesEnd && current != oldRange.second; ++valuesBegin, ++current) {
        current->second = *valuesBegin;
    }

    // if there were more nodes than we need to insert then erase remaining ones
    for (; current != oldRange.second; erase(current++)) {
    }

    // if there were less nodes than we need to insert then emplace the rest of the range
    if (valuesBegin != valuesEnd) {
        const std::string keyStr = std::string(key);
        for (; valuesBegin != valuesEnd; ++valuesBegin) {
            emplace_hint(oldRange.second, keyStr, std::string(*valuesBegin));
        }
    }
}

/** TQuickCgiParam is a faster non-editable version of TCgiParameters.
 * Care should be taken when replacing:
 *  - note that the result of Get() is invalidated when TQuickCgiParam object is destroyed.
 */

class TQuickCgiParam: public std::multimap<std::string_view, std::string_view> {
public:
    TQuickCgiParam() = default;

    explicit TQuickCgiParam(const std::string_view cgiParamStr);

    Y_PURE_FUNCTION
    bool Has(const std::string_view name, const std::string_view value) const noexcept;

    Y_PURE_FUNCTION
    bool Has(const std::string_view name) const noexcept {
        const auto pair = equal_range(static_cast<std::string>(name));
        return pair.first != pair.second;
    }

    Y_PURE_FUNCTION
    const std::string_view& Get(const std::string_view name, size_t numOfValue = 0) const noexcept;

private:
    std::string UnescapeBuf;
};
