#pragma once

#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <memory>

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
    std::unique_ptr<TImpl> Impl_;
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
    std::unique_ptr<TImpl> Impl_;
};

/** @} */
