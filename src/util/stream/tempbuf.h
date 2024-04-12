#pragma once

#include "output.h"

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/stream/tempbuf.h
#include <ydb-cpp-sdk/util/memory/tempbuf.h>
========
#include <src/util/memory/tempbuf.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/stream/tempbuf.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/stream/tempbuf.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TTempBufOutput: public IOutputStream, public TTempBuf {
public:
    inline TTempBufOutput() = default;

    explicit TTempBufOutput(size_t size)
        : TTempBuf(size)
    {
    }

    TTempBufOutput(TTempBufOutput&&) noexcept = default;
    TTempBufOutput& operator=(TTempBufOutput&&) noexcept = default;

protected:
    void DoWrite(const void* data, size_t len) override;
};
