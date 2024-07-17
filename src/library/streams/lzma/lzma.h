#pragma once

#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/zerocopy.h>

#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <memory>

class TLzmaCompress: public IOutputStream {
public:
    TLzmaCompress(IOutputStream* slave, size_t level = 7);
    ~TLzmaCompress() override;

private:
    void DoWrite(const void* buf, size_t len) override;
    void DoFinish() override;

private:
    class TImpl;
    std::unique_ptr<TImpl> Impl_;
};

class TLzmaDecompress: public IInputStream {
public:
    TLzmaDecompress(IInputStream* slave);
    TLzmaDecompress(IZeroCopyInput* input);
    ~TLzmaDecompress() override;

private:
    size_t DoRead(void* buf, size_t len) override;

private:
    class TImpl;
    class TImplStream;
    class TImplZeroCopy;
    std::unique_ptr<TImpl> Impl_;
};
