#pragma once

#include <ydb-cpp-sdk/util/digest/numeric.h>
#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/string_hash.h>
#include <ydb-cpp-sdk/util/generic/typetraits.h>
#include <ydb-cpp-sdk/util/memory/alloc.h>
#include <ydb-cpp-sdk/util/system/compat.h>

#include <cstring>
#include <functional>
#include <string>
#include <typeindex>
#include <utility>

namespace std {
    template <>
    struct less<const char*> {
        bool operator()(const char* x, const char* y) const {
            return std::strcmp(x, y) < 0;
        }
    };
    template <>
    struct equal_to<const char*> {
        bool operator()(const char* x, const char* y) const {
            return std::strcmp(x, y) == 0;
        }
        using is_transparent = void;
    };
}

namespace NHashPrivate {
    template <class T, bool needNumericHashing>
    struct THashHelper {
        inline size_t operator()(const T& t) const noexcept {
            return static_cast<size_t>(t); // If you have a compilation error here, look at explanation below:
            // Probably error is caused by undefined template specialization of THash<T>
            // You can find examples of specialization in this file
        }
    };

    template <class T>
    struct THashHelper<T, true> {
        inline size_t operator()(const T& t) const noexcept {
            return NumericHash(t);
        }
    };

    template <typename C>
    struct TStringHash {
        using is_transparent = void;

        inline size_t operator()(const std::basic_string_view<C> s) const noexcept {
            return NHashPrivate::ComputeStringHash(s.data(), s.size());
        }
    };
}

template <class T>
struct hash: public NHashPrivate::THashHelper<T, std::is_scalar<T>::value && !std::is_integral<T>::value> {
};

template <typename T>
struct hash<const T*> {
    inline size_t operator()(const T* t) const noexcept {
        return NumericHash(t);
    }
};

template <class T>
struct hash<T*>: public ::hash<const T*> {
};

template <>
struct hash<const char*>: ::NHashPrivate::TStringHash<char> {
};

template <size_t n>
struct hash<char[n]>: ::NHashPrivate::TStringHash<char> {
};

template <>
struct THash<std::string_view>: ::NHashPrivate::TStringHash<char> {
};

// template <>
// struct hash<std::string>: ::NHashPrivate::TStringHash<char> {
// };

// template <>
// struct hash<std::u16string>: ::NHashPrivate::TStringHash<wchar16> {
// };

template <>
struct THash<std::u16string_view>: ::NHashPrivate::TStringHash<wchar16> {
};

// template <>
// struct hash<std::u32string>: ::NHashPrivate::TStringHash<wchar32> {
// };

template <>
struct THash<std::u32string_view>: ::NHashPrivate::TStringHash<wchar32> {
};

template <class C, class T, class A>
struct hash<std::basic_string<C, T, A>>: ::NHashPrivate::TStringHash<C> {
};

template <>
struct THash<std::type_index> {
    inline size_t operator()(const std::type_index& index) const {
        return index.hash_code();
    }
};

namespace NHashPrivate {
    template <typename T>
    Y_FORCE_INLINE static size_t HashObject(const T& val) {
        return THash<T>()(val);
    }

    template <size_t I, bool IsLastElement, typename... TArgs>
    struct TupleHashHelper {
        Y_FORCE_INLINE static size_t Hash(const std::tuple<TArgs...>& tuple) {
            return CombineHashes(HashObject(std::get<I>(tuple)),
                                 TupleHashHelper<I + 1, I + 2 >= sizeof...(TArgs), TArgs...>::Hash(tuple));
        }
    };

    template <size_t I, typename... TArgs>
    struct TupleHashHelper<I, true, TArgs...> {
        Y_FORCE_INLINE static size_t Hash(const std::tuple<TArgs...>& tuple) {
            return HashObject(std::get<I>(tuple));
        }
    };

}

template <typename... TArgs>
struct THash<std::tuple<TArgs...>> {
    size_t operator()(const std::tuple<TArgs...>& tuple) const {
        return NHashPrivate::TupleHashHelper<0, 1 >= sizeof...(TArgs), TArgs...>::Hash(tuple);
    }
};

template <class T>
struct THash: public ::hash<T> {
};

namespace NHashPrivate {
    template <class TFirst, class TSecond, bool IsEmpty = std::is_empty<THash<TFirst>>::value&& std::is_empty<THash<TSecond>>::value>
    struct TPairHash {
    private:
        THash<TFirst> FirstHash;
        THash<TSecond> SecondHash;

    public:
        template <class T>
        inline size_t operator()(const T& pair) const {
            return CombineHashes(FirstHash(pair.first), SecondHash(pair.second));
        }
    };

    /**
     * Specialization for the case where both hash functors are empty. Basically the
     * only one we care about. We don't introduce additional specializations for
     * cases where only one of the functors is empty as the code bloat is just not worth it.
     */
    template <class TFirst, class TSecond>
    struct TPairHash<TFirst, TSecond, true> {
        template <class T>
        inline size_t operator()(const T& pair) const {
            // maps have TFirst = const TFoo, which would make for an undefined specialization
            using TFirstClean = std::remove_cv_t<TFirst>;
            using TSecondClean = std::remove_cv_t<TSecond>;
            return CombineHashes(THash<TFirstClean>()(pair.first), THash<TSecondClean>()(pair.second));
        }
    };
}

template <class TFirst, class TSecond>
struct hash<std::pair<TFirst, TSecond>>: public NHashPrivate::TPairHash<TFirst, TSecond> {
};

template <class T>
struct TEqualTo: public std::equal_to<T> {
};

template <>
struct TEqualTo<std::string>: public TEqualTo<std::string_view> {
    using is_transparent = void;
};

template <>
struct TEqualTo<std::u16string>: public TEqualTo<std::u16string_view> {
    using is_transparent = void;
};

template <>
struct TEqualTo<std::u32string>: public TEqualTo<std::u32string_view> {
    using is_transparent = void;
};

template <class TFirst, class TSecond>
struct TEqualTo<std::pair<TFirst, TSecond>> {
    template <class TOther>
    inline bool operator()(const std::pair<TFirst, TSecond>& a, const TOther& b) const {
        return TEqualTo<TFirst>()(a.first, b.first) && TEqualTo<TSecond>()(a.second, b.second);
    }
    using is_transparent = void;
};

template <class T>
struct TCIEqualTo {
};

template <>
struct TCIEqualTo<const char*> {
    inline bool operator()(const char* a, const char* b) const {
        return stricmp(a, b) == 0;
    }
};

template <>
struct TCIEqualTo<std::string> {
    inline bool operator()(const std::string& a, const std::string& b) const {
        return a.size() == b.size() && strnicmp(a.data(), b.data(), a.size()) == 0;
    }
};

template <class T>
struct TLess: public std::less<T> {
};

template <>
struct TLess<std::u16string>: public TLess<std::u16string_view> {
    using is_transparent = void;
};

template <>
struct TLess<std::u32string>: public TLess<std::u32string_view> {
    using is_transparent = void;
};

template <class T>
struct TGreater: public std::greater<T> {
};

template <>
struct TGreater<std::u16string>: public TGreater<std::u16string_view> {
    using is_transparent = void;
};

template <>
struct TGreater<std::u32string>: public TGreater<std::u32string_view> {
    using is_transparent = void;
};
