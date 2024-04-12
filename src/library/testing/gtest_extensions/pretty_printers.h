#pragma once

#include <src/util/generic/variant.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/str.h>
#include <ydb-cpp-sdk/util/string/escape.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/stream/output.h>
#include <src/util/stream/str.h>
#include <src/util/string/escape.h>
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <string_view>

/**
 * Automatically define GTest pretty printer for type that can print itself to util's `IOutputStream`.
 *
 * Note that this macro should be instantiated in the same namespace as the type you're printing, otherwise
 * ADL will not find it.
 *
 * Example:
 *
 * We define a struct `TMyContainer` and an output operator that works with `IOutputStream`. We then use this macro
 * to automatically define GTest pretty printer:
 *
 * ```
 * namespace NMy {
 *     struct TMyContainer {
 *         int x, y;
 *     };
 * }
 *
 * template <>
 * inline void Out<NMy::TMyContainer>(IOutputStream& stream, TTypeTraits<NMy::TMyContainer>::TFuncParam value) {
 *     stream << "{ x=" << value.x << ", y=" << value.y << " }";
 * }
 *
 * namespace NMy {
 *     Y_GTEST_ARCADIA_PRINTER(TMyContainer)
 * }
 * ```
 */
#define Y_GTEST_ARCADIA_PRINTER(T) \
    void PrintTo(const T& value, std::ostream* stream) {   \
        ::std::string ss;                \
        ::TStringOutput s{ss};       \
        s << value;                  \
        *stream << ss;               \
    }


template <typename TCharType, typename TCharTraits>
void PrintTo(const std::basic_string<TCharType, TCharTraits>& value, std::ostream* stream) {
    *stream << NUtils::Quote(value);
}

// template <typename TCharType, typename TCharTraits>
// void PrintTo(std::basic_string_view<TCharType, TCharTraits> value, std::ostream* stream) {
//     *stream << NUtils::Quote(std::basic_string<TCharType, TCharTraits>{value});
// }

template <typename T>
void PrintTo(const std::optional<T>& value, std::ostream* stream) {
    if (value.has_value()) {
        ::testing::internal::UniversalPrint(value.value(), stream);
    } else {
        *stream << "nothing";
    }
}

inline void PrintTo(std::nullopt_t /* value */, std::ostream* stream) {
    *stream << "nothing";
}

inline void PrintTo(std::monostate /* value */, std::ostream* stream) {
    *stream << "monostate";
}

inline void PrintTo(TInstant value, std::ostream* stream) {
    *stream << value.ToString();
}

inline void PrintTo(TDuration value, std::ostream* stream) {
    *stream << value.ToString();
}
