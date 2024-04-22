
#include <src/library/testing/gtest/gtest.h>

#include <src/library/yt/memory/safe_memory_reader.h>

namespace NYT {
namespace {

////////////////////////////////////////////////////////////////////////////////

TEST(TSafeMemoryReaderTest, Simple)
{
    TSafeMemoryReader reader;

    int i = 1;

    int value;
    ASSERT_TRUE(reader.Read(&i, &value));
    ASSERT_EQ(value, 1);

    ASSERT_FALSE(reader.Read(reinterpret_cast<void*>(0x1ee3beef), &value));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace
} // namespace NYT
