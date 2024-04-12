#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/string/builder.h
#include <ydb-cpp-sdk/util/stream/str.h>
========
#include <src/util/stream/str.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/string/builder.h
#include <utility>

namespace NPrivateStringBuilder {
    class TStringBuilder: public std::string {
    public:
        inline TStringBuilder()
            : Out(*this)
        {
        }

        TStringBuilder(TStringBuilder&& rhs) noexcept
            : std::string(std::move(rhs))
            , Out(*this)
        {
        }

        TStringOutput Out;
    };

    template <class T>
    static inline TStringBuilder& operator<<(TStringBuilder& builder Y_LIFETIME_BOUND, const T& t) {
        builder.Out << t;

        return builder;
    }

    template <class T>
    static inline TStringBuilder&& operator<<(TStringBuilder&& builder Y_LIFETIME_BOUND, const T& t) {
        builder.Out << t;

        return std::move(builder);
    }
}

using TStringBuilder = NPrivateStringBuilder::TStringBuilder;
