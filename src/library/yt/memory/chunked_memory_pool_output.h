#pragma once

#include "public.h"
#include "ref.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/zerocopy_output.h>

#include <ydb-cpp-sdk/util/generic/size_literals.h>
=======
#include <src/util/stream/zerocopy_output.h>

#include <src/util/generic/size_literals.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/stream/zerocopy_output.h>

#include <ydb-cpp-sdk/util/generic/size_literals.h>
>>>>>>> origin/main

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

