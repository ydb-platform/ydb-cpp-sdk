#pragma once

#include "public.h"
#include "ref.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/zerocopy_output.h>

#include <ydb-cpp-sdk/util/generic/size_literals.h>
=======
#include <src/util/stream/zerocopy_output.h>

#include <src/util/generic/size_literals.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

class TChunkedMemoryPoolOutput
    : public IZeroCopyOutput
{
public:
    static constexpr size_t DefaultChunkSize = 4_KB;

    explicit TChunkedMemoryPoolOutput(
        TChunkedMemoryPool* pool,
        size_t chunkSize = DefaultChunkSize);

    std::vector<TMutableRef> Finish();

private:
    size_t DoNext(void** ptr) override;
    void DoUndo(size_t size) override;

private:
    TChunkedMemoryPool* const Pool_;
    const size_t ChunkSize_;

    char* Begin_ = nullptr;
    char* Current_ = nullptr;
    char* End_ = nullptr;
    std::vector<TMutableRef> Refs_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

