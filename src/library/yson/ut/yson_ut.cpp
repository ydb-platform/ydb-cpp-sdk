#include <src/library/yson/parser.h>
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/library/yson/writer.h>

#include <src/library/testing/gtest/gtest.h>

#include <ydb-cpp-sdk/util/stream/mem.h>
<<<<<<< HEAD
=======
#include <src/library/yson/writer.h>

#include <src/library/testing/gtest/gtest.h>

#include <src/util/stream/mem.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

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
