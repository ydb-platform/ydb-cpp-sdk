#pragma once

#include "cast.h"
#include "split.h"

#include <util/generic/map.h>
#include <util/generic/strbuf.h>
#include <util/generic/string.h>
#include <util/generic/vector.h>
#include <util/string/cast.h>
#include <util/system/yassert.h>

#define KEEP_EMPTY_TOKENS 0x01

//
// NOTE: Check StringSplitter below to get more convenient split string interface.

namespace NPrivate {

    void SplitStringImpl(std::vector<std::string>* res, const char* ptr,
                         const char* delimiter, size_t maxFields, int options);
    void SplitStringImpl(std::vector<std::string>* res, const char* ptr, size_t len,
                         const char* delimiter, size_t maxFields, int options);

    void SplitStringImpl(std::vector<TUtf16String>* res, const wchar16* ptr,
                         const wchar16* delimiter, size_t maxFields, int options);
    void SplitStringImpl(std::vector<TUtf16String>* res, const wchar16* ptr, size_t len,
                         const wchar16* delimiter, size_t maxFields, int options);

    template <typename C>
    struct TStringDeducer;

    template <>
    struct TStringDeducer<char> {
        using type = TString;
    };

    template <>
    struct TStringDeducer<wchar16> {
        using type = TUtf16String;
    };
}

template <typename C>
std::vector<typename ::NPrivate::TStringDeducer<C>::type>
SplitString(const C* ptr, const C* delimiter,
            size_t maxFields = 0, int options = 0) {
    std::vector<typename ::NPrivate::TStringDeducer<C>::type> res;
    ::NPrivate::SplitStringImpl(&res, ptr, delimiter, maxFields, options);
    return res;
}

template <typename C>
std::vector<typename ::NPrivate::TStringDeducer<C>::type>
SplitString(const C* ptr, size_t len, const C* delimiter,
            size_t maxFields = 0, int options = 0) {
    std::vector<typename ::NPrivate::TStringDeducer<C>::type> res;
    ::NPrivate::SplitStringImpl(&res, ptr, len, delimiter, maxFields, options);
    return res;
}

template <typename C>
std::vector<typename ::NPrivate::TStringDeducer<C>::type>
SplitString(const typename ::NPrivate::TStringDeducer<C>::type& str, const C* delimiter,
            size_t maxFields = 0, int options = 0) {
    return SplitString(str.data(), str.size(), delimiter, maxFields, options);
}

template <class TIter>
inline std::string JoinStrings(TIter begin, TIter end, const std::string_view delim) {
    if (begin == end)
        return TString();

    std::string result = ToString(*begin);

    for (++begin; begin != end; ++begin) {
        result.append(delim);
        result.append(ToString(*begin));
    }

    return result;
}

template <class TIter>
inline TUtf16String JoinStrings(TIter begin, TIter end, const std::u16string_view delim) {
    if (begin == end)
        return TUtf16String();

    TUtf16String result = ToWtring(*begin);

    for (++begin; begin != end; ++begin) {
        result.append(delim);
        result.append(ToWtring(*begin));
    }

    return result;
}

/// Concatenates elements of given std::vector<std::string>.
inline std::string JoinStrings(const std::vector<std::string>& v, const std::string_view delim) {
    return JoinStrings(v.begin(), v.end(), delim);
}

inline std::string JoinStrings(const std::vector<std::string>& v, size_t index, size_t count, const std::string_view delim) {
    Y_ASSERT(index + count <= v.size() && "JoinStrings(): index or count out of range");
    return JoinStrings(v.begin() + index, v.begin() + index + count, delim);
}

template <typename T>
inline std::string JoinVectorIntoString(const std::vector<T>& v, const std::string_view delim) {
    return JoinStrings(v.begin(), v.end(), delim);
}

template <typename T>
inline std::string JoinVectorIntoString(const std::vector<T>& v, size_t index, size_t count, const std::string_view delim) {
    Y_ASSERT(index + count <= v.size() && "JoinVectorIntoString(): index or count out of range");
    return JoinStrings(v.begin() + index, v.begin() + index + count, delim);
}

TUtf16String JoinStrings(const std::vector<TUtf16String>& v, const std::u16string_view delim);
TUtf16String JoinStrings(const std::vector<TUtf16String>& v, size_t index, size_t count, const std::u16string_view delim);

//! Converts vector of strings to vector of type T variables
template <typename T, typename TStringType>
std::vector<T> Scan(const std::vector<TStringType>& input) {
    std::vector<T> output;
    output.reserve(input.size());
    for (int i = 0; i < input.ysize(); ++i) {
        output.push_back(FromString<T>(input[i]));
    }
    return output;
}
