#include <src/library/testing/gtest/gtest.h>

#include <src/library/testing/gtest_extensions/assertions.h>

#include <src/library/yt/memory/ref.h>

namespace NYT::NYson {
namespace {

////////////////////////////////////////////////////////////////////////////////

TEST(TSharedRefTest, Save)
{
    const TSharedRef expected = TSharedRef::FromString("My tests data");
    std::stringStream s;
    ::Save(&s, expected);  // only Save supported for TSharedRef. You can ::Load serialized data to vector.
}

////////////////////////////////////////////////////////////////////////////////

} // namespace
} // namespace NYT::NYson
