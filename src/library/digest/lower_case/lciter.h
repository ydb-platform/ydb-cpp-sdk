#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/typetraits.h>
#include <ydb-cpp-sdk/util/string/ascii.h>
=======
#include <src/util/generic/typetraits.h>
#include <src/util/string/ascii.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <iterator>

template <class T>
struct TLowerCaseIterator: public std::iterator<std::input_iterator_tag, T> {
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
