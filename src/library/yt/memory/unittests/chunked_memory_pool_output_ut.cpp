#include <src/library/testing/gtest/gtest.h>

#include <src/library/yt/memory/chunked_memory_pool.h>
#include <src/library/yt/memory/chunked_memory_pool_output.h>

namespace NYT {
namespace {

////////////////////////////////////////////////////////////////////////////////

TEST(TChunkedMemoryPoolOutputTest, Basic)
{
    constexpr size_t PoolChunkSize = 10;
    constexpr size_t PoolOutputChunkSize = 7;
    TChunkedMemoryPool pool(NullRefCountedTypeCookie, PoolChunkSize);
    TChunkedMemoryPoolOutput output(&pool, PoolOutputChunkSize);

    std::string s1("Short.");
    output.Write(s1);

    std::string s2("Quite a long string.");
    output.Write(s2);

    char* buf;
    auto len = output.Next(&buf);
    output.Undo(len);

    auto chunks = output.Finish();
    std::string s;
    for (auto chunk : chunks) {
        s += std::string(chunk.Begin(), chunk.End());
    }
    ASSERT_EQ(s1 + s2, s);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace
} // namespace NYT
