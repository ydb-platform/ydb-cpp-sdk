#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/stream/output.h>
#include <src/util/stream/input.h>
#include <src/util/generic/ptr.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

/**
 * @addtogroup Streams_Archs
 * @{
 */

/**
 * Snappy compressing stream.
 *
 * @see http://code.google.com/p/snappy/
 */
class TSnappyCompress: public IOutputStream {
public:
    TSnappyCompress(IOutputStream* slave, ui16 maxBlockSize = 1 << 15);
    ~TSnappyCompress() override;

private:
    void DoWrite(const void* buf, size_t len) override;
    void DoFlush() override;
    void DoFinish() override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

/**
 * Snappy decompressing stream.
 *
 * @see http://code.google.com/p/snappy/
 */
class TSnappyDecompress: public IInputStream {
public:
    TSnappyDecompress(IInputStream* slave);
    ~TSnappyDecompress() override;

private:
    size_t DoRead(void* buf, size_t len) override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

/** @} */
