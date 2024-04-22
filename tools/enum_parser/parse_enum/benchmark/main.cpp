#include <tools/enum_parser/parse_enum/benchmark/enum.h_serialized.h>
#include <src/library/testing/benchmark/bench.h>
#include <ydb-cpp-sdk/util/generic/algorithm.h>
#include <src/util/generic/vector.h>
#include <ydb-cpp-sdk/util/generic/xrange.h>
#include <src/util/stream/null.h>
#include <ydb-cpp-sdk/util/string/cast.h>

namespace {

    template <class TEnum>
    std::vector<TEnum> SelectValues(size_t count) {
        auto values = GetEnumAllValues<TEnum>().Materialize();
        SortBy(values, [](const TEnum& v) { return IntHash(static_cast<ui64>(v)); });
        values.crop(count);
        return values;
    }

    template <class TEnum>
    std::vector<std::string_view> SelectStrings(size_t count) {
        std::vector<std::string_view> strings;
        strings.reserve(GetEnumItemsCount<TEnum>());
        for (const auto& [_, s] : GetEnumNames<TEnum>()) {
            strings.push_back(s);
        }
        SortBy(strings, [](const std::string_view& s) { return THash<std::string_view>()(s); });
        strings.crop(count);
        return strings;
    }

    template <class TEnum, class TContext>
    void BMToString(TContext& iface) {
        const auto values = SelectValues<TEnum>(5u);
        for (const auto iter : xrange(iface.Iterations())) {
            Y_UNUSED(iter);
            for (const auto value : values) {
                Y_DO_NOT_OPTIMIZE_AWAY(ToString(value).size());
            }
        }
    }

    template <class TEnum, class TContext>
    void BMOut(TContext& iface) {
        const auto values = SelectValues<TEnum>(5u);
        TNullOutput null;
        for (const auto iter : xrange(iface.Iterations())) {
            Y_UNUSED(iter);
            for (const auto value : values) {
                Y_DO_NOT_OPTIMIZE_AWAY(null << value);
            }
        }
    }

    template <class TEnum, class TContext>
    void BMFromString(TContext& iface) {
        const auto strings = SelectStrings<TEnum>(5u);
        for (const auto iter : xrange(iface.Iterations())) {
            Y_UNUSED(iter);
            for (const auto s : strings) {
                Y_DO_NOT_OPTIMIZE_AWAY(FromString<TEnum>(s));
            }
        }
    }

    template <class TEnum, class TContext>
    void BMTryFromString(TContext& iface) {
        auto strings = SelectStrings<TEnum>(5u);
        strings.back() = "fake";

        TEnum value;
        for (const auto iter : xrange(iface.Iterations())) {
            Y_UNUSED(iter);
            for (const auto s : strings) {
                Y_DO_NOT_OPTIMIZE_AWAY(TryFromString<TEnum>(s, value));
            }
        }
    }
}

#define DEFINE_BENCHMARK(name)                     \
    Y_CPU_BENCHMARK(ToString_##name, iface) {      \
        BMToString<name>(iface);                   \
    }                                              \
    Y_CPU_BENCHMARK(Out_##name, iface) {           \
        BMOut<name>(iface);                        \
    }                                              \
    Y_CPU_BENCHMARK(FromString_##name, iface) {    \
        BMFromString<name>(iface);                 \
    }                                              \
    Y_CPU_BENCHMARK(TryFromString_##name, iface) { \
        BMTryFromString<name>(iface);              \
    }

DEFINE_BENCHMARK(ESmallSortedEnum);
DEFINE_BENCHMARK(ESmalUnsortedEnum);
DEFINE_BENCHMARK(EBigSortedEnum);
DEFINE_BENCHMARK(EBigUnsortedEnum);
DEFINE_BENCHMARK(EBigUnsortedDenseEnum);
