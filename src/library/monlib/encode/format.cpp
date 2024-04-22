#include <ydb-cpp-sdk/library/monlib/encode/format.h>

#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

#include <src/util/string/ascii.h>
#include <src/util/string/split.h>
#include <src/util/string/strip.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/string/cast.h>

namespace NMonitoring {
    static ECompression CompressionFromHeader(std::string_view value) {
        if (value.empty()) {
            return ECompression::UNKNOWN;
        }

        for (const auto& it : StringSplitter(value).Split(',').SkipEmpty()) {
            std::string_view token = StripString(it.Token());

            if (AsciiEqualsIgnoreCase(token, NFormatContentEncoding::IDENTITY)) {
                return ECompression::IDENTITY;
            } else if (AsciiEqualsIgnoreCase(token, NFormatContentEncoding::ZLIB)) {
                return ECompression::ZLIB;
            } else if (AsciiEqualsIgnoreCase(token, NFormatContentEncoding::LZ4)) {
                return ECompression::LZ4;
            } else if (AsciiEqualsIgnoreCase(token, NFormatContentEncoding::ZSTD)) {
                return ECompression::ZSTD;
            }
        }

        return ECompression::UNKNOWN;
    }

    static EFormat FormatFromHttpMedia(std::string_view value) {
        if (AsciiEqualsIgnoreCase(value, NFormatContenType::SPACK)) {
            return EFormat::SPACK;
        } else if (AsciiEqualsIgnoreCase(value, NFormatContenType::JSON)) {
            return EFormat::JSON;
        } else if (AsciiEqualsIgnoreCase(value, NFormatContenType::PROTOBUF)) {
            return EFormat::PROTOBUF;
        } else if (AsciiEqualsIgnoreCase(value, NFormatContenType::TEXT)) {
            return EFormat::TEXT;
        } else if (AsciiEqualsIgnoreCase(value, NFormatContenType::PROMETHEUS)) {
            return EFormat::PROMETHEUS;
        } else if (AsciiEqualsIgnoreCase(value, NFormatContenType::UNISTAT)) {
            return EFormat::UNISTAT;
        }

        return EFormat::UNKNOWN;
    }

    EFormat FormatFromAcceptHeader(std::string_view value) {
        EFormat result{EFormat::UNKNOWN};

        for (const auto& it : StringSplitter(value).Split(',').SkipEmpty()) {
            std::string_view token = NUtils::Before(StripString(it.Token()), ';');

            result = FormatFromHttpMedia(token);
            if (result != EFormat::UNKNOWN) {
                break;
            }
        }

        return result;
    }

    EFormat FormatFromContentType(std::string_view value) {
        value = NUtils::NextTok(value, ';');

        return FormatFromHttpMedia(value);
    }

    std::string_view ContentTypeByFormat(EFormat format) {
        switch (format) {
            case EFormat::SPACK:
                return NFormatContenType::SPACK;
            case EFormat::JSON:
                return NFormatContenType::JSON;
            case EFormat::PROTOBUF:
                return NFormatContenType::PROTOBUF;
            case EFormat::TEXT:
                return NFormatContenType::TEXT;
            case EFormat::PROMETHEUS:
                return NFormatContenType::PROMETHEUS;
            case EFormat::UNISTAT:
                return NFormatContenType::UNISTAT;
            case EFormat::UNKNOWN:
                return std::string_view();
        }

        Y_ABORT(); // for GCC
    }

    ECompression CompressionFromAcceptEncodingHeader(std::string_view value) {
        return CompressionFromHeader(value);
    }

    ECompression CompressionFromContentEncodingHeader(std::string_view value) {
        return CompressionFromHeader(value);
    }

    std::string_view ContentEncodingByCompression(ECompression compression) {
        switch (compression) {
            case ECompression::IDENTITY:
                return NFormatContentEncoding::IDENTITY;
            case ECompression::ZLIB:
                return NFormatContentEncoding::ZLIB;
            case ECompression::LZ4:
                return NFormatContentEncoding::LZ4;
            case ECompression::ZSTD:
                return NFormatContentEncoding::ZSTD;
            case ECompression::UNKNOWN:
                return std::string_view();
        }

        Y_ABORT(); // for GCC
    }

}

template <>
NMonitoring::EFormat FromStringImpl<NMonitoring::EFormat>(const char* str, size_t len) {
    using NMonitoring::EFormat;
    std::string_view value(str, len);
    if (value == std::string_view("SPACK")) {
        return EFormat::SPACK;
    } else if (value == std::string_view("JSON")) {
        return EFormat::JSON;
    } else if (value == std::string_view("PROTOBUF")) {
        return EFormat::PROTOBUF;
    } else if (value == std::string_view("TEXT")) {
        return EFormat::TEXT;
    } else if (value == std::string_view("PROMETHEUS")) {
        return EFormat::PROMETHEUS;
    } else if (value == std::string_view("UNISTAT")) {
        return EFormat::UNISTAT;
    } else if (value == std::string_view("UNKNOWN")) {
        return EFormat::UNKNOWN;
    }
    ythrow yexception() << "unknown format: " << value;
}

template <>
void Out<NMonitoring::EFormat>(IOutputStream& o, NMonitoring::EFormat f) {
    using NMonitoring::EFormat;
    switch (f) {
        case EFormat::SPACK:
            o << std::string_view("SPACK");
            return;
        case EFormat::JSON:
            o << std::string_view("JSON");
            return;
        case EFormat::PROTOBUF:
            o << std::string_view("PROTOBUF");
            return;
        case EFormat::TEXT:
            o << std::string_view("TEXT");
            return;
        case EFormat::PROMETHEUS:
            o << std::string_view("PROMETHEUS");
            return;
        case EFormat::UNISTAT:
            o << std::string_view("UNISTAT");
            return;
        case EFormat::UNKNOWN:
            o << std::string_view("UNKNOWN");
            return;
    }

    Y_ABORT(); // for GCC
}

template <>
NMonitoring::ECompression FromStringImpl<NMonitoring::ECompression>(const char* str, size_t len) {
    using NMonitoring::ECompression;
    std::string_view value(str, len);
    if (value == std::string_view("IDENTITY")) {
        return ECompression::IDENTITY;
    } else if (value == std::string_view("ZLIB")) {
        return ECompression::ZLIB;
    } else if (value == std::string_view("LZ4")) {
        return ECompression::LZ4;
    } else if (value == std::string_view("ZSTD")) {
        return ECompression::ZSTD;
    } else if (value == std::string_view("UNKNOWN")) {
        return ECompression::UNKNOWN;
    }
    ythrow yexception() << "unknown compression: " << value;
}

template <>
void Out<NMonitoring::ECompression>(IOutputStream& o, NMonitoring::ECompression c) {
    using NMonitoring::ECompression;
    switch (c) {
        case ECompression::IDENTITY:
            o << std::string_view("IDENTITY");
            return;
        case ECompression::ZLIB:
            o << std::string_view("ZLIB");
            return;
        case ECompression::LZ4:
            o << std::string_view("LZ4");
            return;
        case ECompression::ZSTD:
            o << std::string_view("ZSTD");
            return;
        case ECompression::UNKNOWN:
            o << std::string_view("UNKNOWN");
            return;
    }

    Y_ABORT(); // for GCC
}
