#include "zstd.h"

#include <library/cpp/testing/unittest/registar.h>

#include <util/random/fast.h>
#include <util/stream/null.h>
#include <util/stream/str.h>

Y_UNIT_TEST_SUITE(TZstdTestSuite) {
    std::string Compress(std::string data, int quality = -1) {
        std::string compressed;
        std::stringOutput output(compressed);
        TZstdCompress compressStream(&output, quality);
        compressStream.Write(data.data(), data.size());
        compressStream.Finish();
        output.Finish();
        return compressed;
    }

    std::string Decompress(std::string data) {
        std::stringInput input(data);
        TZstdDecompress decompressStream(&input);
        return decompressStream.ReadAll();
    }

    void TestCase(const std::string& s) {
        UNIT_ASSERT_VALUES_EQUAL(s, Decompress(Compress(s, -1)));
        UNIT_ASSERT_VALUES_EQUAL(s, Decompress(Compress(s, 0)));
        UNIT_ASSERT_VALUES_EQUAL(s, Decompress(Compress(s, 22)));
        UNIT_ASSERT_VALUES_EQUAL(s, Decompress(Compress(s, 11)));
        UNIT_ASSERT_VALUES_EQUAL(s, Decompress(Compress(s, 100500)));
    }

    std::string GenerateRandomString(size_t size) {
        TReallyFastRng32 rng(42);
        std::string result;
        result.reserve(size + sizeof(ui64));
        while (result.size() < size) {
            ui64 value = rng.GenRand64();
            result += std::string_view(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        result.resize(size);
        return result;
    }

    Y_UNIT_TEST(TestHelloWorld) {
        TestCase("hello world");
    }

    Y_UNIT_TEST(TestSeveralStreamsWithSameQuality) {
        auto s1 = GenerateRandomString(1 << 15);
        auto s2 = GenerateRandomString(1 << 15);
        auto c1 = Compress(s1);
        auto c2 = Compress(s2);
        UNIT_ASSERT_VALUES_EQUAL(s1 + s2, Decompress(c1 + c2));
    }

    Y_UNIT_TEST(TestSeveralStreamsWithDifferentQuality) {
        auto s1 = GenerateRandomString(1 << 15);
        auto s2 = GenerateRandomString(1 << 15);
        auto c1 = Compress(s1, 1);
        auto c2 = Compress(s2, 2);
        UNIT_ASSERT_VALUES_EQUAL(s1 + s2, Decompress(c1 + c2));
    }

    Y_UNIT_TEST(TestIncompleteStream) {
        std::string manyAs(64 * 1024, 'a');
        auto compressed = Compress(manyAs);
        std::string truncated(compressed.data(), compressed.size() - 1);
        UNIT_CHECK_GENERATED_EXCEPTION(Decompress(truncated), std::exception);
    }

    Y_UNIT_TEST(Test64KB) {
        auto manyAs = std::string(64 * 1024, 'a');
        std::string str("Hello from the Matrix!@#% How are you?}{\n\t\a");
        TestCase(manyAs + str + manyAs);
    }

    Y_UNIT_TEST(Test1MB) {
        TestCase(GenerateRandomString(1 * 1024 * 1024));
    }

    Y_UNIT_TEST(TestEmpty) {
        TestCase("");
    }

    Y_UNIT_TEST(TestWriteAfterFinish) {
        TNullOutput output;
        TZstdCompress compressStream(&output);
        compressStream.Finish();
        UNIT_ASSERT_EXCEPTION_CONTAINS(compressStream.Write("a", 1), std::exception, "Cannot use stream after finish.");
        UNIT_ASSERT_EXCEPTION_CONTAINS(compressStream.Flush(), std::exception, "Cannot use stream after finish.");
    }
}
