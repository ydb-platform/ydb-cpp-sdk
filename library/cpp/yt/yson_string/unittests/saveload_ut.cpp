#include <library/cpp/testing/gtest/gtest.h>

#include <library/cpp/testing/gtest_extensions/assertions.h>

#include <library/cpp/yt/yson_string/string.h>

namespace NYT::NYson {
namespace {

////////////////////////////////////////////////////////////////////////////////

TEST(TYsonStringTest, SaveLoadNull)
{
    const TYsonString expected;
    std::stringStream s;
    ::Save(&s, expected);
    TYsonString result;
    ::Load(&s, result);
    EXPECT_EQ(expected, result);
}

TEST(TYsonStringTest, SaveLoadString)
{
    const TYsonString expected(std::string("My tests data"));
    std::stringStream s;
    ::Save(&s, expected);
    TYsonString result;
    ::Load(&s, result);
    EXPECT_EQ(expected, result);
}

TEST(TYsonStringTest, SaveLoadSharedRef)
{
    TSharedRef ref = TSharedRef::FromString("My tests data");
    const TYsonString expected(ref);
    std::stringStream s;
    ::Save(&s, expected);
    TYsonString result;
    ::Load(&s, result);
    EXPECT_EQ(expected, result);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace
} // namespace NYT::NYson
