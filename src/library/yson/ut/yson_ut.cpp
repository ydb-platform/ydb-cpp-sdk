#include <src/library/yson/parser.h>
#include <src/library/yson/writer.h>

#include <src/library/testing/gtest/gtest.h>

#include <src/util/stream/mem.h>

using namespace NYson;

TEST(TTestYson, YT_17658)
{
   const auto data = std::string_view{"\x01\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc"};

    auto input = TMemoryInput{data};

    std::stringStream out;
    {
        TYsonWriter writer(&out, EYsonFormat::Text);
        EXPECT_THROW(ParseYsonStringBuffer(data, &writer), std::exception);
    }
}
