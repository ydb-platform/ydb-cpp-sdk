#pragma once

#include "string_builder.h"

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/library/yt/exception/exception.h>

#include <ydb-cpp-sdk/util/datetime/base.h>

#include <string>

#include <ydb-cpp-sdk/util/string/strip.h>
<<<<<<< HEAD
=======
#include <src/library/yt/exception/exception.h>

#include <src/util/datetime/base.h>

#include <string>

#include <src/util/string/strip.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

#include <vector>
#include <set>
#include <map>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

//! Formatters enable customizable way to turn an object into a string.
//! This default implementation uses |FormatValue|.
struct TDefaultFormatter
{
    template <class T>
    void operator()(TStringBuilderBase* builder, const T& obj) const
    {
        FormatValue(builder, obj, std::string_view("v"));
    }
};

static constexpr std::string_view DefaultJoinToStringDelimiter = ", ";
static constexpr std::string_view DefaultKeyValueDelimiter = ": ";
static constexpr std::string_view DefaultRangeEllipsisFormat = "...";

// ASCII characters from 0x20 = ' ' to 0x7e = '~' are printable.
static constexpr char PrintableASCIILow = 0x20;
static constexpr char PrintableASCIIHigh = 0x7e;
static constexpr std::string_view IntToHexLowercase = "0123456789abcdef";
static constexpr std::string_view IntToHexUppercase = "0123456789ABCDEF";

//! Joins a range of items into a string intermixing them with the delimiter.
/*!
 *  \param builder String builder where the output goes.
 *  \param begin Iterator pointing to the first item (inclusive).
 *  \param end Iterator pointing to the last item (not inclusive).
 *  \param formatter Formatter to apply to the items.
 *  \param delimiter A delimiter to be inserted between items: ", " by default.
 *  \return The resulting combined string.
 */
template <class TIterator, class TFormatter>
void JoinToString(
    TStringBuilderBase* builder,
    const TIterator& begin,
    const TIterator& end,
    const TFormatter& formatter,
    std::string_view delimiter = DefaultJoinToStringDelimiter)
{
    for (auto current = begin; current != end; ++current) {
        if (current != begin) {
            builder->AppendString(delimiter);
        }
        formatter(builder, *current);
    }
}

template <class TIterator, class TFormatter>
std::string JoinToString(
    const TIterator& begin,
    const TIterator& end,
    const TFormatter& formatter,
    std::string_view delimiter = DefaultJoinToStringDelimiter)
{
    TStringBuilder builder;
    JoinToString(&builder, begin, end, formatter, delimiter);
    return builder.Flush();
}

//! A handy shortcut with default formatter.
template <class TIterator>
std::string JoinToString(
    const TIterator& begin,
    const TIterator& end,
    std::string_view delimiter = DefaultJoinToStringDelimiter)
{
    return JoinToString(begin, end, TDefaultFormatter(), delimiter);
}

//! Joins a collection of given items into a string intermixing them with the delimiter.
/*!
 *  \param collection A collection containing the items to be joined.
 *  \param formatter Formatter to apply to the items.
 *  \param delimiter A delimiter to be inserted between items; ", " by default.
 */
template <class TCollection, class TFormatter>
std::string JoinToString(
    const TCollection& collection,
    const TFormatter& formatter,
    std::string_view delimiter = DefaultJoinToStringDelimiter)
{
    using std::begin;
    using std::end;
    return JoinToString(begin(collection), end(collection), formatter, delimiter);
}

//! A handy shortcut with the default formatter.
template <class TCollection>
std::string JoinToString(
    const TCollection& collection,
    std::string_view delimiter = DefaultJoinToStringDelimiter)
{
    return JoinToString(collection, TDefaultFormatter(), delimiter);
}

//! Concatenates a bunch of std::string_view-like instances into std::string.
template <class... Ts>
std::string ConcatToString(Ts... args)
{
    size_t length = 0;
    ((length += args.length()), ...);

    std::string result;
    result.reserve(length);
    (result.append(args), ...);

    return result;
}

//! Converts a range of items into strings.
template <class TIter, class TFormatter>
std::vector<std::string> ConvertToStrings(
    const TIter& begin,
    const TIter& end,
    const TFormatter& formatter,
    size_t maxSize = std::numeric_limits<size_t>::max())
{
    std::vector<std::string> result;
    for (auto it = begin; it != end; ++it) {
        TStringBuilder builder;
        formatter(&builder, *it);
        result.push_back(builder.Flush());
        if (result.size() == maxSize) {
            break;
        }
    }
    return result;
}

//! A handy shortcut with the default formatter.
template <class TIter>
std::vector<std::string> ConvertToStrings(
    const TIter& begin,
    const TIter& end,
    size_t maxSize = std::numeric_limits<size_t>::max())
{
    return ConvertToStrings(begin, end, TDefaultFormatter(), maxSize);
}

//! Converts a given collection of items into strings.
/*!
 *  \param collection A collection containing the items to be converted.
 *  \param formatter Formatter to apply to the items.
 *  \param maxSize Size limit for the resulting vector.
 */
template <class TCollection, class TFormatter>
std::vector<std::string> ConvertToStrings(
    const TCollection& collection,
    const TFormatter& formatter,
    size_t maxSize = std::numeric_limits<size_t>::max())
{
    using std::begin;
    using std::end;
    return ConvertToStrings(begin(collection), end(collection), formatter, maxSize);
}

//! A handy shortcut with default formatter.
template <class TCollection>
std::vector<std::string> ConvertToStrings(
    const TCollection& collection,
    size_t maxSize = std::numeric_limits<size_t>::max())
{
    return ConvertToStrings(collection, TDefaultFormatter(), maxSize);
}

////////////////////////////////////////////////////////////////////////////////

void UnderscoreCaseToCamelCase(TStringBuilderBase* builder, std::string_view str);
std::string UnderscoreCaseToCamelCase(std::string_view str);

void CamelCaseToUnderscoreCase(TStringBuilderBase* builder, std::string_view str);
std::string CamelCaseToUnderscoreCase(std::string_view str);

std::string TrimLeadingWhitespaces(const std::string& str);
std::string Trim(const std::string& str, const std::string& whitespaces);

////////////////////////////////////////////////////////////////////////////////

//! Implemented for |[u]i(32|64)|.
template <class T>
char* WriteDecIntToBufferBackwards(char* ptr, T value);

//! Implemented for |[u]i(32|64)|.
template <class T>
char* WriteHexIntToBufferBackwards(char* ptr, T value, bool uppercase);

////////////////////////////////////////////////////////////////////////////////

struct TCaseInsensitiveStringHasher
{
    size_t operator()(std::string_view arg) const;
};

struct TCaseInsensitiveStringEqualityComparer
{
    bool operator()(std::string_view lhs, std::string_view rhs) const;
};

////////////////////////////////////////////////////////////////////////////////

bool TryParseBool(std::string_view value, bool* result);
bool ParseBool(std::string_view value);
std::string_view FormatBool(bool value);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
