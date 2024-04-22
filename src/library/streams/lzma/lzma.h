#pragma once

#include <src/util/stream/input.h>
#include <src/util/stream/output.h>
#include <src/util/stream/zerocopy.h>

#include <ydb-cpp-sdk/util/generic/ptr.h>

class TLzmaCompress: public IOutputStream {
public:
    TLzmaCompress(IOutputStream* slave, size_t level = 7);
    ~TLzmaCompress() override;

private:
    void DoWrite(const void* buf, size_t len) override;
    void DoFinish() override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
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
    THolder<TImpl> Impl_;
};
