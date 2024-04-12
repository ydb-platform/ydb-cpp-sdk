#pragma once

#include "ref.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/zerocopy.h>
=======
#include <src/util/stream/zerocopy.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

class TChunkedInputStream
    : public IZeroCopyInput
{
public:
    explicit TChunkedInputStream(std::vector<TSharedRef> blocks);

    size_t DoNext(const void** ptr, size_t len) override;

private:
    const std::vector<TSharedRef> Blocks_;
    size_t Index_ = 0;
    size_t Position_ = 0;

    void SkipCompletedBlocks();
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
