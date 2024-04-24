#pragma once

#include <ydb-cpp-sdk/util/generic/typetraits.h>
#include <src/util/string/cast.h>
#include "cast.h"

/*
 * Default implementation of AppendToString uses a temporary std::string object which is inefficient. You can overload it
 * for your type to speed up string joins. If you already have an Out() or operator<<() implementation you can simply
 * do the following:
 *
 *      inline void AppendToString(std::string& dst, const TMyType& t) {
 *          TStringOutput o(dst);
 *          o << t;
 *      }
 *
 * Unfortunately we can't do this by default because for some types ToString() is defined while Out() is not.
 * For standard types (strings of all kinds and arithmetic types) we don't use a temporary std::string in AppendToString().
 */

template <typename TCharType, typename T>
inline std::enable_if_t<!std::is_arithmetic<std::remove_cv_t<T>>::value, void>
AppendToString(std::basic_string<TCharType>& dst, const T& t) {
    dst.append(ToString(t));
}

template <typename TCharType, typename T>
inline std::enable_if_t<std::is_arithmetic<std::remove_cv_t<T>>::value, void>
AppendToString(std::basic_string<TCharType>& dst, const T& t) {
    char buf[512];
    dst.append(buf, ToString<std::remove_cv_t<T>>(t, buf, sizeof(buf)));
}

template <typename TCharType>
inline void AppendToString(std::basic_string<TCharType>& dst, const TCharType* t) {
    dst.append(t);
}

template <typename TCharType>
inline void AppendToString(std::basic_string<TCharType>& dst, std::basic_string_view<TCharType> t) {
    dst.append(t);
}

namespace NPrivate {
    template <typename T>
    inline size_t GetLength(const T&) {
        // By default don't pre-allocate space when joining and appending non-string types.
        // This code can be extended by estimating stringified length for specific types (e.g. 10 for ui32).
        return 0;
    }

    template <>
    inline size_t GetLength(const std::string& s) {
        return s.length();
    }

    template <>
    inline size_t GetLength(const std::string_view& s) {
        return s.length();
    }

    template <>
    inline size_t GetLength(const char* const& s) {
        return (s ? std::char_traits<char>::length(s) : 0);
    }

    inline size_t GetAppendLength(const std::string_view /*delim*/) {
        return 0;
    }

    template <typename TFirst, typename... TRest>
    size_t GetAppendLength(const std::string_view delim, const TFirst& f, const TRest&... r) {
        return delim.length() + ::NPrivate::GetLength(f) + ::NPrivate::GetAppendLength(delim, r...);
    }
}

template <typename TCharType>
inline void AppendJoinNoReserve(std::basic_string<TCharType>&, std::basic_string_view<TCharType>) {
}

template <typename TCharType, typename TFirst, typename... TRest>
inline void AppendJoinNoReserve(std::basic_string<TCharType>& dst, std::basic_string_view<TCharType> delim, const TFirst& f, const TRest&... r) {
    AppendToString(dst, delim);
    AppendToString(dst, f);
    AppendJoinNoReserve(dst, delim, r...);
}

template <typename... TValues>
inline void AppendJoin(std::string& dst, const std::string_view delim, const TValues&... values) {
    const size_t appendLength = ::NPrivate::GetAppendLength(delim, values...);
    if (appendLength > 0) {
        dst.reserve(dst.length() + appendLength);
    }
    AppendJoinNoReserve(dst, delim, values...);
}

template <typename TFirst, typename... TRest>
inline std::string Join(const std::string_view delim, const TFirst& f, const TRest&... r) {
    std::string ret = ToString(f);
    AppendJoin(ret, delim, r...);
    return ret;
}

// Note that char delimeter @cdelim will be printed as single char string,
// but any char value @v will be printed as corresponding numeric code.
// For example, Join('a', 'a', 'a') will print "97a97" (see unit-test).
template <typename... TValues>
inline std::string Join(char cdelim, const TValues&... v) {
    return Join(std::string_view(&cdelim, 1), v...);
}

namespace NPrivate {
    template <typename TCharType, typename TIter>
    inline std::basic_string<TCharType> JoinRange(std::basic_string_view<TCharType> delim, const TIter beg, const TIter end) {
        std::basic_string<TCharType> out;
        if (beg != end) {
            size_t total = ::NPrivate::GetLength(*beg);
            for (TIter pos = beg; ++pos != end;) {
                total += delim.length() + ::NPrivate::GetLength(*pos);
            }
            if (total > 0) {
                out.reserve(total);
            }

            AppendToString(out, *beg);
            for (TIter pos = beg; ++pos != end;) {
                AppendJoinNoReserve(out, delim, *pos);
            }
        }

        return out;
    }

} // namespace NPrivate

template <typename TIter>
std::string JoinRange(std::string_view delim, const TIter beg, const TIter end) {
    return ::NPrivate::JoinRange<char>(delim, beg, end);
}

template <typename TIter>
std::string JoinRange(char delim, const TIter beg, const TIter end) {
    std::string_view delimBuf(&delim, 1);
    return ::NPrivate::JoinRange<char>(delimBuf, beg, end);
}

template <typename TIter>
std::u16string JoinRange(std::u16string_view delim, const TIter beg, const TIter end) {
    return ::NPrivate::JoinRange<wchar16>(delim, beg, end);
}

template <typename TIter>
std::u16string JoinRange(wchar16 delim, const TIter beg, const TIter end) {
    std::u16string_view delimBuf(&delim, 1);
    return ::NPrivate::JoinRange<wchar16>(delimBuf, beg, end);
}

template <typename TIter>
std::u32string JoinRange(std::u32string_view delim, const TIter beg, const TIter end) {
    return ::NPrivate::JoinRange<wchar32>(delim, beg, end);
}

template <typename TIter>
std::u32string JoinRange(wchar32 delim, const TIter beg, const TIter end) {
    std::u32string_view delimBuf(&delim, 1);
    return ::NPrivate::JoinRange<wchar32>(delimBuf, beg, end);
}

template <typename TCharType, typename TContainer>
inline std::basic_string<TCharType> JoinSeq(std::basic_string_view<TCharType> delim, const TContainer& data) {
    using std::begin;
    using std::end;
    return JoinRange(delim, begin(data), end(data));
}

template <typename TCharType, typename TContainer>
inline std::basic_string<TCharType> JoinSeq(const TCharType* delim, const TContainer& data) {
    std::basic_string_view<TCharType> delimBuf = delim;
    return JoinSeq(delimBuf, data);
}

template <typename TCharType, typename TContainer>
inline std::basic_string<TCharType> JoinSeq(const std::basic_string<TCharType>& delim, const TContainer& data) {
    std::basic_string_view<TCharType> delimBuf = delim;
    return JoinSeq(delimBuf, data);
}

template <typename TCharType, typename TContainer>
inline std::enable_if_t<
    std::is_same_v<TCharType, char> ||
        std::is_same_v<TCharType, char16_t> ||
        std::is_same_v<TCharType, char32_t>,
    std::basic_string<TCharType>>
JoinSeq(TCharType delim, const TContainer& data) {
    std::basic_string_view<TCharType> delimBuf(&delim, 1);
    return JoinSeq(delimBuf, data);
}

/** \brief Functor for streaming iterative objects from TIterB e to TIterE b, separated with delim.
 *         Difference from JoinSeq, JoinRange, Join is the lack of std::string object - all depends on operator<< for the type and
 *         realization of IOutputStream
 */
template <class TIterB, class TIterE>
struct TRangeJoiner {
    friend constexpr IOutputStream& operator<<(IOutputStream& stream, const TRangeJoiner<TIterB, TIterE>& rangeJoiner) {
        if (rangeJoiner.b != rangeJoiner.e) {
            stream << *rangeJoiner.b;

            for (auto it = std::next(rangeJoiner.b); it != rangeJoiner.e; ++it)
                stream << rangeJoiner.delim << *it;
        }
        return stream;
    }

    constexpr TRangeJoiner(std::string_view delim, TIterB&& b, TIterE&& e)
        : delim(delim)
        , b(std::forward<TIterB>(b))
        , e(std::forward<TIterE>(e))
    {
    }

private:
    const std::string_view delim;
    const TIterB b;
    const TIterE e;
};

template <class TIterB, class TIterE = TIterB>
constexpr auto MakeRangeJoiner(std::string_view delim, TIterB&& b, TIterE&& e) {
    return TRangeJoiner<TIterB, TIterE>(delim, std::forward<TIterB>(b), std::forward<TIterE>(e));
}

template <class TContainer>
constexpr auto MakeRangeJoiner(std::string_view delim, const TContainer& data) {
    return MakeRangeJoiner(delim, std::cbegin(data), std::cend(data));
}

template <class TVal>
constexpr auto MakeRangeJoiner(std::string_view delim, const std::initializer_list<TVal>& data) {
    return MakeRangeJoiner(delim, std::cbegin(data), std::cend(data));
}

/* We force (std::initializer_list<std::string_view>) input type for (std::string) and (const char*) types because:
 * # When (std::initializer_list<std::string>) is used, std::string objects are copied into the initializer_list object.
 *   Storing std::string_views instead is faster, even with COW-enabled strings.
 * # For (const char*) we calculate length only once and store it in std::string_view. Otherwise strlen scan would be executed
 *   in both GetAppendLength and AppendToString. For string literals constant lengths get propagated in compile-time.
 *
 * This way JoinSeq(",", { s1, s2 }) always does the right thing whatever types s1 and s2 have.
 *
 * If someone needs to join std::initializer_list<std::string> -- it still works because of the TContainer template above.
 */

template <typename T>
inline std::enable_if_t<
    !std::is_same<std::decay_t<T>, std::string>::value && !std::is_same<std::decay_t<T>, const char*>::value,
    std::string>
JoinSeq(const std::string_view delim, const std::initializer_list<T>& data) {
    return JoinRange(delim, data.begin(), data.end());
}

inline std::string JoinSeq(const std::string_view delim, const std::initializer_list<std::string_view>& data) {
    return JoinRange(delim, data.begin(), data.end());
}
