#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/stream/zerocopy.h
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/generic/ylimits.h>
========
#include <src/util/system/yassert.h>
#include <src/util/system/defaults.h>
#include <src/util/generic/ylimits.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/stream/zerocopy.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/stream/zerocopy.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include "input.h"

class IOutputStream;

/**
 * @addtogroup Streams
 * @{
 */

/**
 * Input stream with direct access to the input buffer.
 *
 * Derived classes must implement `DoNext` method.
 */
class IZeroCopyInput: public IInputStream {
public:
    IZeroCopyInput() noexcept = default;
    ~IZeroCopyInput() override;

    IZeroCopyInput(IZeroCopyInput&&) noexcept = default;
    IZeroCopyInput& operator=(IZeroCopyInput&&) noexcept = default;

    /**
     * Returns the next data chunk from this input stream.
     *
     * Note that this function is not guaranteed to return the requested number
     * of bytes, even if they are in fact available in the stream.
     *
     * @param ptr[out]                  Pointer to the start of the data chunk.
     * @param len[in]                   Maximal size of the data chunk to be returned, in bytes.
     * @returns                         Size of the returned data chunk, in bytes.
     *                                  Return value of zero signals end of stream.
     */
    template <class T>
    inline size_t Next(T** ptr, size_t len) {
        Y_ASSERT(ptr);

        return DoNext((const void**)ptr, len);
    }

    template <class T>
    inline size_t Next(T** ptr) {
        return Next(ptr, Max<size_t>());
    }

protected:
    size_t DoRead(void* buf, size_t len) override;
    size_t DoSkip(size_t len) override;
    ui64 DoReadAll(IOutputStream& out) override;
    virtual size_t DoNext(const void** ptr, size_t len) = 0;
};

/**
 * Input stream with direct access to the input buffer and ability to undo read
 *
 * Derived classes must implement `DoUndo` method.
 */
class IZeroCopyInputFastReadTo: public IZeroCopyInput {
public:
    IZeroCopyInputFastReadTo() noexcept = default;
    ~IZeroCopyInputFastReadTo() override;

    IZeroCopyInputFastReadTo(IZeroCopyInputFastReadTo&&) noexcept = default;
    IZeroCopyInputFastReadTo& operator=(IZeroCopyInputFastReadTo&&) noexcept = default;

protected:
    size_t DoReadTo(std::string& st, char ch) override;

private:
    /**
     * Undo read.
     *
     * Note that this function not check if you try undo more that read. In fact Undo used for undo read in last chunk.
     *
     * @param len[in]                   Bytes to undo.
     */
    inline void Undo(size_t len) {
        if (len) {
            DoUndo(len);
        }
    }
    virtual void DoUndo(size_t len) = 0;
};

/** @} */
