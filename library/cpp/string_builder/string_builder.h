#pragma once

#include <library/cpp/string_utils/string_output/string_output.h>

#include <string>

namespace NUtils {

class TYdbStringBuilder: public std::string {
public:
    inline TYdbStringBuilder()
        : Out(*this)
    {
    }

    TYdbStringBuilder(TYdbStringBuilder&& rhs) noexcept
        : std::string(std::move(rhs))
        , Out(*this)
    {
    }

    TStringOutput Out;
};

template <class T>
static inline TYdbStringBuilder& operator<<(TYdbStringBuilder& builder Y_LIFETIME_BOUND, const T& t) {
    builder.Out << t;

    return builder;
}

template <class T>
static inline TYdbStringBuilder&& operator<<(TYdbStringBuilder&& builder Y_LIFETIME_BOUND, const T& t) {
    builder.Out << t;

    return std::move(builder);
}

}

