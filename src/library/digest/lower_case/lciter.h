#pragma once

#include <ydb-cpp-sdk/util/generic/typetraits.h>
#include <ydb-cpp-sdk/util/string/ascii.h>

#include <iterator>

template <class T>
struct TLowerCaseIterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    using TNonConst = std::remove_const_t<T>;

    inline TLowerCaseIterator(T* c)
        : C(c)
    {
    }

    inline TLowerCaseIterator& operator++() noexcept {
        ++C;

        return *this;
    }

    inline TLowerCaseIterator operator++(int) noexcept {
        return C++;
    }

    inline TNonConst operator*() const noexcept {
        return AsciiToLower(*C);
    }

    T* C;
};

template <class T>
inline bool operator==(const TLowerCaseIterator<T>& l, const TLowerCaseIterator<T>& r) noexcept {
    return l.C == r.C;
}

template <class T>
inline bool operator!=(const TLowerCaseIterator<T>& l, const TLowerCaseIterator<T>& r) noexcept {
    return !(l == r);
}
