#include "diff.h"

#include <src/library/testing/unittest/registar.h>

using namespace NDiff;

struct TDiffTester {
    std::stringStream Res;
    std::vector<TChunk<char>> Chunks;

    std::string_view Special(const std::string_view& str) const {
        return str;
    }

    std::string_view Common(const std::span<const char>& str) const {
        return std::string_view(str.begin(), str.end());
    }

    std::string_view Left(const std::span<const char>& str) const {
        return std::string_view(str.begin(), str.end());
    }

    std::string_view Right(const std::span<const char>& str) const {
        return std::string_view(str.begin(), str.end());
    }

    void Test(const std::string_view& a, const std::string_view& b, const std::string& delims = " \t\n") {
        Chunks.clear();
        InlineDiff(Chunks, a, b, delims);
        Res.clear();
        PrintChunks(Res, *this, Chunks);
    }

    const std::string& Result() const {
        return Res.Str();
    }
};

Y_UNIT_TEST_SUITE(DiffTokens) {
    Y_UNIT_TEST(ReturnValue) {
        std::vector<TChunk<char>> res;
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "aaa", "aaa"), 0);
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "aaa", "aa"), 1);
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "aaa", "a"), 2);
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "aaa", "abc"), 2);
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "aaa", "aba"), 1);
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "", "aba"), 3);
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "aaa", "aaaa"), 1);
        UNIT_ASSERT_VALUES_EQUAL(InlineDiff(res, "abc", "xyz"), 3);
    }

    Y_UNIT_TEST(EqualStringsOneToken) {
        TDiffTester tester;

        tester.Test("aaa", "aaa");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "aaa");
    }

    Y_UNIT_TEST(NonCrossingStringsOneToken) {
        TDiffTester tester;

        tester.Test("aaa", "bbb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|bbb)");

        tester.Test("aaa", "bbbb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|bbbb)");
    }

    Y_UNIT_TEST(Simple) {
        TDiffTester tester;

        tester.Test("aaa", "abb", "");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "a(aa|bb)");

        tester.Test("aac", "abc", "");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "a(a|b)c");

        tester.Test("123", "133", "");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "1(2|3)3");

        tester.Test("[1, 2, 3]", "[1, 3, 3]", "");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "[1, (2|3), 3]");
    }

    Y_UNIT_TEST(CommonCharOneToken) {
        TDiffTester tester;

        tester.Test("abcde", "accfg");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(abcde|accfg)");
    }

    Y_UNIT_TEST(EqualStringsTwoTokens) {
        TDiffTester tester;

        std::string_view str("aaa bbb");
        tester.Test(str, str);

        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "aaa bbb");
    }

    Y_UNIT_TEST(NonCrossingStringsTwoTokens) {
        TDiffTester tester;

        tester.Test("aaa bbb", "ccc ddd");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|ccc) (bbb|ddd)");

        tester.Test("aaa bbb", "c d");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|c) (bbb|d)");
    }

    Y_UNIT_TEST(SimpleTwoTokens) {
        TDiffTester tester;

        tester.Test("aaa ccd", "abb cce");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|abb) (ccd|cce)");

        tester.Test("aac cbb", "aa bb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aac|aa) (cbb|bb)");
    }

    Y_UNIT_TEST(MixedTwoTokens) {
        TDiffTester tester;

        tester.Test("aaa bbb", "bbb aaa");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(|bbb )aaa( bbb|)");

        tester.Test("aaa bbb", " bbb aaa");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|) bbb(| aaa)");

        tester.Test(" aaa bbb ", " bbb aaa ");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(| bbb) aaa (bbb |)");

        tester.Test("aaa bb", " bbb aa");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|) (bb|bbb aa)");
    }

    Y_UNIT_TEST(TwoTokensInOneString) {
        TDiffTester tester;

        tester.Test("aaa bbb", "aaa");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "aaa( bbb|)");

        tester.Test("aaa bbb", "aaa ");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "aaa (bbb|)");

        tester.Test("aaa bbb", " bbb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa|) bbb");

        tester.Test("aaa bbb", "bbb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "(aaa |)bbb");
    }

    Y_UNIT_TEST(Multiline) {
        TDiffTester tester;

        tester.Test("aaa\nabc\nbbb", "aaa\nacc\nbbb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "aaa\n(abc|acc)\nbbb");

        tester.Test("aaa\nabc\nbbb", "aaa\nac\nbbb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "aaa\n(abc|ac)\nbbb");
    }

    Y_UNIT_TEST(DifferentDelimiters) {
        TDiffTester tester;

        tester.Test("aaa bbb", "aaa\tbbb");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "aaa( |\t)bbb");

        tester.Test(" aaa\tbbb\n", "\taaa\nbbb ");
        //~ std::cerr << tester.Result() << std::endl;
        UNIT_ASSERT_VALUES_EQUAL(tester.Result(), "( |\t)aaa(\t|\n)bbb(\n| )");
    }
}
