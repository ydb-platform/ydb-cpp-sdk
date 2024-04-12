#include "node_io.h"

#include <src/library/testing/unittest/registar.h>

#include <src/util/stream/mem.h>

using namespace NYson;

namespace {
    void GenerateDeepJson(std::stringStream& stream, ui64 depth) {
        stream << "{\"key\":";
        for (ui32 i = 0; i < depth - 1; ++i) {
            stream << "[";
        }
        for (ui32 i = 0; i < depth - 1; ++i) {
            stream << "]";
        }
        stream << "}";
    }
}

Y_UNIT_TEST_SUITE(TestNodeFromJsonStringIterativeTest) {
    Y_UNIT_TEST(NoCrashOn1e5Brackets) {
        constexpr ui32 brackets = static_cast<ui32>(1e5);

        std::stringStream jsonStream;
        GenerateDeepJson(jsonStream, brackets);

        UNIT_ASSERT_EXCEPTION(
            NYT::NodeFromJsonStringIterative(jsonStream.Str()),
            std::exception);
    }

    Y_UNIT_TEST(NoCrashOn1025Brackets) {
        constexpr ui32 brackets = 1025;

        std::stringStream jsonStream;
        GenerateDeepJson(jsonStream, brackets);

        UNIT_ASSERT_EXCEPTION(
            NYT::NodeFromJsonStringIterative(jsonStream.Str()),
            std::exception);
    }

    Y_UNIT_TEST(NoErrorOn1024Brackets) {
        constexpr ui32 brackets = 1024;

        std::stringStream jsonStream;
        GenerateDeepJson(jsonStream, brackets);

        UNIT_ASSERT_NO_EXCEPTION(NYT::NodeFromJsonStringIterative(jsonStream.Str()));
    }
}
