#include <src/library/yson/parser.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/yson/writer.h>

#include <src/library/testing/gtest/gtest.h>

#include <ydb-cpp-sdk/util/stream/mem.h>
=======
#include <src/library/yson/writer.h>

#include <src/library/testing/gtest/gtest.h>

#include <src/util/stream/mem.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

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
