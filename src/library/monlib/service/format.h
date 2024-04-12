#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/encode/format.h>

#include <ydb-cpp-sdk/util/string/ascii.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/generic/typetraits.h>
=======
#include <src/library/monlib/encode/format.h>

#include <src/util/string/ascii.h>
#include <src/util/generic/yexception.h>
#include <src/util/generic/typetraits.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NMonitoring {
    namespace NPrivate {
        Y_HAS_MEMBER(Name, Name);
        Y_HAS_MEMBER(second, Second);
    } // namespace NPrivate

    template <typename TRequest>
    ECompression ParseCompression(const TRequest& req) {
        auto&& headers = req.GetHeaders();

        constexpr auto isPlainPair = NPrivate::THasSecond<std::decay_t<decltype(*headers.begin())>>::value;

        auto it = FindIf(std::begin(headers), std::end(headers),
            [=] (const auto& h) {
                if constexpr (NPrivate::THasName<std::decay_t<decltype(h)>>::value) {
                    return AsciiCompareIgnoreCase(h.Name(), std::string_view("accept-encoding")) == 0;
                } else if (isPlainPair) {
                    return AsciiCompareIgnoreCase(h.first, std::string_view("accept-encoding")) == 0;
                }
            });

        if (it == std::end(headers)) {
            return NMonitoring::ECompression::IDENTITY;
        }

        NMonitoring::ECompression val{};
        if constexpr (isPlainPair) {
            val = CompressionFromAcceptEncodingHeader(it->second);
        } else {
            val = CompressionFromAcceptEncodingHeader(it->Value());
        }

        return val != NMonitoring::ECompression::UNKNOWN
            ? val
            : NMonitoring::ECompression::IDENTITY;
    }

    template <typename TRequest>
    NMonitoring::EFormat ParseFormat(const TRequest& req) {
        auto&& formatStr = req.GetParams()
            .Get(std::string_view("format"));

        if (!formatStr.empty()) {
            if (formatStr == std::string_view("SPACK")) {
                return EFormat::SPACK;
            } else if (formatStr == std::string_view("TEXT")) {
                return EFormat::TEXT;
            } else if (formatStr == std::string_view("JSON")) {
                return EFormat::JSON;
            } else {
                ythrow yexception() << "unknown format: " << formatStr << ". Only spack is supported here";
            }
        }

        auto&& headers = req.GetHeaders();
        constexpr auto isPlainPair = NPrivate::THasSecond<std::decay_t<decltype(*headers.begin())>>::value;

        auto it = FindIf(std::begin(headers), std::end(headers),
            [=] (const auto& h) {
                if constexpr (NPrivate::THasName<std::decay_t<decltype(h)>>::value) {
                    return AsciiCompareIgnoreCase(h.Name(), std::string_view("accept")) == 0;
                } else if (isPlainPair) {
                    return AsciiCompareIgnoreCase(h.first, std::string_view("accept")) == 0;
                }
            });

        if (it != std::end(headers)) {
            if constexpr (isPlainPair) {
                return FormatFromAcceptHeader(it->second);
            } else {
                return FormatFromAcceptHeader(it->Value());
            }
        }

        return EFormat::UNKNOWN;
    }

} // namespace NMonitoring
