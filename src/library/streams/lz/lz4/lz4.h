#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/stream/output.h>
#include <src/util/stream/input.h>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

/**
 * @addtogroup Streams_Archs
 * @{
 */

/**
 * Lz4 compressing stream.
 *
 * @see http://code.google.com/p/lz4/
 */
class TLz4Compress: public IOutputStream {
public:
    TLz4Compress(IOutputStream* slave, ui16 maxBlockSize = 1 << 15);
    ~TLz4Compress() override;

private:
    void DoWrite(const void* buf, size_t len) override;
    void DoFlush() override;
    void DoFinish() override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

/**
 * Lz4 decompressing stream.
 *
 * @see http://code.google.com/p/lz4/
 */
class TLz4Decompress: public IInputStream {
public:
    TLz4Decompress(IInputStream* slave);
    ~TLz4Decompress() override;

private:
    size_t DoRead(void* buf, size_t len) override;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

/** @} */
