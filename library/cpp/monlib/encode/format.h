#pragma once

#include <util/generic/strbuf.h>

namespace NMonitoring {
    namespace NFormatContenType {
        constexpr std::string_view TEXT = "application/x-solomon-txt";
        constexpr std::string_view JSON = "application/json";
        constexpr std::string_view PROTOBUF = "application/x-solomon-pb";
        constexpr std::string_view SPACK = "application/x-solomon-spack";
        constexpr std::string_view PROMETHEUS = "text/plain";
        constexpr std::string_view UNISTAT = "text/json";
    }

    namespace NFormatContentEncoding {
        constexpr std::string_view IDENTITY = "identity";
        constexpr std::string_view ZLIB = "zlib";
        constexpr std::string_view LZ4 = "lz4";
        constexpr std::string_view ZSTD = "zstd";
    }

    /**
     * Defines format types for metric encoders.
     */
    enum class EFormat {
        /**
         * Special case when it was not possible to determine format.
         */
        UNKNOWN,

        /**
         * Read more https://wiki.yandex-team.ru/solomon/api/dataformat/spackv1
         */
        SPACK,

        /**
         * Read more https://wiki.yandex-team.ru/solomon/api/dataformat/json
         */
        JSON,

        /**
         * Read more https://wiki.yandex-team.ru/golovan/userdocs/stat-handle
         */
        UNISTAT,

        /**
         * Simple protobuf format, only for testing purposes.
         */
        PROTOBUF,

        /**
         * Simple text representation, only for debug purposes.
         */
        TEXT,

        /**
         * Prometheus text-based format
         */
        PROMETHEUS,
    };

    /**
     * Defines compression algorithms for metric encoders.
     */
    enum class ECompression {
        /**
         * Special case when it was not possible to determine compression.
         */
        UNKNOWN,

        /**
         * Means no compression.
         */
        IDENTITY,

        /**
         * Using the zlib structure (defined in RFC 1950), with the
         * deflate compression algorithm and Adler32 checkums.
         */
        ZLIB,

        /**
         * Using LZ4 compression algorithm (read http://lz4.org for more info)
         * with XxHash32 checksums.
         */
        LZ4,

        /**
         * Using Zstandard compression algorithm (read http://zstd.net for more
         * info) with XxHash32 checksums.
         */
        ZSTD,
    };

    enum class EMetricsMergingMode {
        /**
         * Do not merge metric batches. If several points of the same metric were
         * added multiple times accross different writes, paste them as
         * separate metrics.
         *
         * Example:
         * COUNTER [(ts1, val1)] |     COUNTER [(ts1, val1)]
         * COUNTER [(ts2, val2)] | --> COUNTER [(ts2, val2)]
         * COUNTER [(ts3, val3)] |     COUNTER [(ts3, val3)]
         */
        DEFAULT,

        /**
         * If several points of the same metric were added multiple times across
         * different writes, merge all values to one timeseries.
         *
         * Example:
         * COUNTER [(ts1, val1)] |
         * COUNTER [(ts2, val2)] | --> COUNTER [(ts1, val1), (ts2, val2), (ts3, val3)]
         * COUNTER [(ts3, val3)] |
         */
        MERGE_METRICS,
    };

    /**
     * Matches serialization format by the given "Accept" header value.
     *
     * @param value  value of the "Accept" header.
     * @return most preffered serialization format type
     */
    EFormat FormatFromAcceptHeader(std::string_view value);

    /**
     * Matches serialization format by the given "Content-Type" header value
     *
     * @param value value of the "Content-Type" header
     * @return message format
     */
    EFormat FormatFromContentType(std::string_view value);

    /**
     * Returns value for "Content-Type" header determined by the given
     * format type.
     *
     * @param format  serialization format type
     * @return mime-type indentificator
     *         or empty string if format is UNKNOWN
     */
    std::string_view ContentTypeByFormat(EFormat format);

    /**
     * Matches compression algorithm by the given "Accept-Encoding" header value.
     *
     * @param value  value of the "Accept-Encoding" header.
     * @return most preffered compression algorithm
     */
    ECompression CompressionFromAcceptEncodingHeader(std::string_view value);

    /**
     * Matches compression algorithm by the given "Content-Encoding" header value.
     *
     * @param value  value of the "Accept-Encoding" header.
     * @return most preffered compression algorithm
     */
    ECompression CompressionFromContentEncodingHeader(std::string_view value);

    /**
     * Returns value for "Content-Encoding" header determined by the given
     * compression algorithm.
     *
     * @param compression  encoding compression alg
     * @return media-type compresion algorithm
     *         or empty string if compression is UNKNOWN
     */
    std::string_view ContentEncodingByCompression(ECompression compression);

}
