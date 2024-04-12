#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/stream/input.h>
#include <src/util/stream/output.h>
#include <src/util/generic/ptr.h>
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#define BZIP_BUF_LEN (8 * 1024)
#define BZIP_COMPRESSION_LEVEL 6

/**
 * @addtogroup Streams_Archs
 * @{
 */

class TBZipException: public yexception {
};

class TBZipDecompressError: public TBZipException {
};

class TBZipCompressError: public TBZipException {
};

class TBZipDecompress: public IInputStream {
public:
    TBZipDecompress(IInputStream* input, size_t bufLen = BZIP_BUF_LEN);
    ~TBZipDecompress() override;

private:
    size_t DoRead(void* buf, size_t size) override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

class TBZipCompress: public IOutputStream {
public:
    TBZipCompress(IOutputStream* out, size_t compressionLevel = BZIP_COMPRESSION_LEVEL, size_t bufLen = BZIP_BUF_LEN);
    ~TBZipCompress() override;

private:
    void DoWrite(const void* buf, size_t size) override;
    void DoFlush() override;
    void DoFinish() override;

public:
    class TImpl;
    THolder<TImpl> Impl_;
};

/** @} */
