#ifndef VARIANT_INL_H_
#error "Direct inclusion of this file is not allowed, include variant.h"
// For the sake of sane code completion.
#include "variant.h"
#endif

#include <type_traits>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

namespace NDetail {

template <size_t Index, class... Ts>
struct TVariantFormatter;

template <size_t Index>
struct TVariantFormatter<Index>
{
    template <class TVariant>
    static void Do(TYdbStringBuilderBase* /*builder*/, const TVariant& /*variant*/, std::string_view /*spec*/)
    { }
};

template <size_t Index, class T, class... Ts>
struct TVariantFormatter<Index, T, Ts...>
{
    template <class TVariant>
    static void Do(TYdbStringBuilderBase* builder, const TVariant& variant, std::string_view spec)
    {
        if (variant.index() == Index) {
            FormatValue(builder, std::get<Index>(variant), spec);
        } else {
            TVariantFormatter<Index + 1, Ts...>::Do(builder, variant, spec);
        }
    }
};

} // namespace NDetail

template <class... Ts>
void FormatValue(TYdbStringBuilderBase* builder, const std::variant<Ts...>& variant, std::string_view spec)
{
    NDetail::TVariantFormatter<0, Ts...>::Do(builder, variant, spec);
}

template <class... Ts>
std::string ToString(const std::variant<Ts...>& variant)
{
    return ToStringViaBuilder(variant);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
