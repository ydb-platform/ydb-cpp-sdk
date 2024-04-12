#pragma once

#include <ydb-cpp-sdk/util/stream/fwd.h>
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include "buffered.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/system/defaults.h>
#include <src/util/generic/ptr.h>
#include <src/util/generic/yexception.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

/**
 * @addtogroup Streams_Archs
 * @{
 */

struct TZLibError: public yexception {
};

struct TZLibCompressorError: public TZLibError {
};

struct TZLibDecompressorError: public TZLibError {
};

namespace ZLib {
    enum StreamType: ui8 {
        Auto = 0, /**< Auto detect format. Can be used for decompression only. */
        ZLib = 1,
        GZip = 2,
        Raw = 3,
        Invalid = 4
    };

    enum {
        ZLIB_BUF_LEN = 8 * 1024
    };
}

/**
 * Non-buffered ZLib decompressing stream.
 *
 * Please don't use `TZLibDecompress` if you read text data from stream using
 * `ReadLine`, it is VERY slow (approx 10 times slower, according to synthetic
 * benchmark).
 */
class TZLibDecompress: public IInputStream {
public:
    TZLibDecompress(IZeroCopyInput* input, ZLib::StreamType type = ZLib::Auto, std::string_view dict = {});
    TZLibDecompress(IInputStream* input, ZLib::StreamType type = ZLib::Auto, size_t buflen = ZLib::ZLIB_BUF_LEN,
                    std::string_view dict = {});

    /**
     * Allows/disallows multiple sequential compressed streams. Allowed by default.
     *
     * If multiple streams are allowed, their decompressed content will be concatenated.
     * If multiple streams are disabled, then only first stream is decompressed. After that end
     * of IInputStream will have happen, i.e. method Read() will return 0.
     *
     * @param allowMultipleStreams - flag to allow (true) or disable (false) multiple streams.
     */
    void SetAllowMultipleStreams(bool allowMultipleStreams);

    ~TZLibDecompress() override;

protected:
    size_t DoRead(void* buf, size_t size) override;

public:
    class TImpl;
    THolder<TImpl> Impl_;
};

/**
 * Non-buffered ZLib compressing stream.
 */
class TZLibCompress: public IOutputStream {
public:
    struct TParams {
        inline TParams(IOutputStream* out)
            : Out(out)
            , Type(ZLib::ZLib)
            , CompressionLevel(6)
            , BufLen(ZLib::ZLIB_BUF_LEN)
        {
        }

        inline TParams& SetType(ZLib::StreamType type) noexcept {
            Type = type;

            return *this;
        }

        inline TParams& SetCompressionLevel(size_t level) noexcept {
            CompressionLevel = level;

            return *this;
        }

        inline TParams& SetBufLen(size_t buflen) noexcept {
            BufLen = buflen;

            return *this;
        }

        inline TParams& SetDict(const std::string_view dict) noexcept {
            Dict = dict;

            return *this;
        }

        IOutputStream* Out;
        ZLib::StreamType Type;
        size_t CompressionLevel;
        size_t BufLen;
        std::string_view Dict;
    };

    inline TZLibCompress(const TParams& params) {
        Init(params);
    }

    inline TZLibCompress(IOutputStream* out, ZLib::StreamType type) {
        Init(TParams(out).SetType(type));
    }

    inline TZLibCompress(IOutputStream* out, ZLib::StreamType type, size_t compression_level) {
        Init(TParams(out).SetType(type).SetCompressionLevel(compression_level));
    }

    inline TZLibCompress(IOutputStream* out, ZLib::StreamType type, size_t compression_level, size_t buflen) {
        Init(TParams(out).SetType(type).SetCompressionLevel(compression_level).SetBufLen(buflen));
    }

    ~TZLibCompress() override;

private:
    void Init(const TParams& opts);

    void DoWrite(const void* buf, size_t size) override;
    void DoFlush() override;
    void DoFinish() override;

public:
    class TImpl;

    /** To allow inline constructors. */
    struct TDestruct {
        static void Destroy(TImpl* impl);
    };

    THolder<TImpl, TDestruct> Impl_;
};
