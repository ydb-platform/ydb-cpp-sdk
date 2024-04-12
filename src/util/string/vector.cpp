#include "util.h"
#include "split.h"
#include "vector.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
=======
#include <src/util/system/defaults.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

template <class TConsumer, class TDelim, typename TChr>
static inline void DoSplit2(TConsumer& c, TDelim& d, const std::basic_string_view<TChr> str, int) {
    SplitString(str.data(), str.data() + str.size(), d, c);
}

template <class TConsumer, class TDelim, typename TChr>
static inline void DoSplit1(TConsumer& cc, TDelim& d, const std::basic_string_view<TChr> str, int opts) {
    if (opts & KEEP_EMPTY_TOKENS) {
        DoSplit2(cc, d, str, opts);
    } else {
        TSkipEmptyTokens<TConsumer> sc(&cc);

        DoSplit2(sc, d, str, opts);
    }
}

template <class C, class TDelim, typename TChr>
static inline void DoSplit0(C* res, const std::basic_string_view<TChr> str, TDelim& d, size_t maxFields, int options) {
    using TStringType = std::conditional_t<std::is_same<TChr, wchar16>::value, std::u16string, std::string>;
    res->clear();

    if (!str.data()) {
        return;
    }

    using TConsumer = TContainerConsumer<C>;
    TConsumer cc(res);

    if (maxFields) {
        TLimitingConsumer<TConsumer, const TChr> lc(maxFields, &cc);

        DoSplit1(lc, d, str, options);

        if (lc.Last) {
            res->push_back(TStringType(lc.Last, str.data() + str.size() - lc.Last));
        }
    } else {
        DoSplit1(cc, d, str, options);
    }
}

template <typename TChr>
static void SplitStringImplT(std::vector<std::conditional_t<std::is_same<TChr, wchar16>::value, std::u16string, std::string>>* res,
                             const std::basic_string_view<TChr> str, const TChr* delim, size_t maxFields, int options) {
    if (!*delim) {
        return;
    }

    if (*(delim + 1)) {
        TStringDelimiter<const TChr> d(delim, std::char_traits<TChr>::length(delim));

        DoSplit0(res, str, d, maxFields, options);
    } else {
        TCharDelimiter<const TChr> d(*delim);

        DoSplit0(res, str, d, maxFields, options);
    }
}

void ::NPrivate::SplitStringImpl(std::vector<std::string>* res, const char* ptr, const char* delim, size_t maxFields, int options) {
    return SplitStringImplT<char>(res, std::string_view(ptr), delim, maxFields, options);
}

void ::NPrivate::SplitStringImpl(std::vector<std::string>* res, const char* ptr, size_t len, const char* delim, size_t maxFields, int options) {
    return SplitStringImplT<char>(res, std::string_view(ptr, len), delim, maxFields, options);
}

void ::NPrivate::SplitStringImpl(std::vector<std::u16string>* res, const wchar16* ptr, const wchar16* delimiter, size_t maxFields, int options) {
    return SplitStringImplT<wchar16>(res, std::u16string_view(ptr), delimiter, maxFields, options);
}

void ::NPrivate::SplitStringImpl(std::vector<std::u16string>* res, const wchar16* ptr, size_t len, const wchar16* delimiter, size_t maxFields, int options) {
    return SplitStringImplT<wchar16>(res, std::u16string_view(ptr, len), delimiter, maxFields, options);
}

std::u16string JoinStrings(const std::vector<std::u16string>& v, const std::u16string_view delim) {
    return JoinStrings(v.begin(), v.end(), delim);
}

std::u16string JoinStrings(const std::vector<std::u16string>& v, size_t index, size_t count, const std::u16string_view delim) {
    const size_t f = Min(index, v.size());
    const size_t l = f + Min(count, v.size() - f);

    return JoinStrings(v.begin() + f, v.begin() + l, delim);
}
