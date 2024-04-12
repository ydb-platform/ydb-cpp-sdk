#include "split.h"

#include <src/library/testing/unittest/registar.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
#include <src/util/charset/wide.h>
#include <src/util/datetime/cputimer.h>

#include <iostream>
#include <string>
#include <string_view>

template <typename T>
static inline void OldSplit(char* pszBuf, T* pRes) {
    pRes->resize(0);
    pRes->push_back(pszBuf);
    for (char* pszData = pszBuf; *pszData; ++pszData) {
        if (*pszData == '\t') {
            *pszData = 0;
            pRes->push_back(pszData + 1);
        }
    }
}

template <class T1, class T2>
inline void Cmp(const T1& t1, const T2& t2) {
    try {
        UNIT_ASSERT_EQUAL(t1.size(), t2.size());
    } catch (...) {
        Print(t1);
        std::cerr << "---------------" << std::endl;
        Print(t2);

        throw;
    }

    auto i = t1.begin();
    auto j = t2.begin();

    for (; i != t1.end() && j != t2.end(); ++i, ++j) {
        try {
            UNIT_ASSERT_EQUAL(*i, *j);
        } catch (...) {
            std::cerr << "(" << *i << ")->(" << *j << ")" << std::endl;

            throw;
        }
    }
}

template <class T>
inline void Print(const T& t) {
    for (typename T::const_iterator i = t.begin(); i != t.end(); ++i) {
        std::cerr << *i << std::endl;
    }
}

template <template <typename> class TConsumer, typename TResult, typename I, typename TDelimiter>
void TestDelimiterOnString(TResult& good, I* str, const TDelimiter& delim) {
    TResult test;
    TConsumer<TResult> consumer(&test);
    SplitString(str, delim, consumer);
    Cmp(good, test);
    UNIT_ASSERT_EQUAL(good, test);
}

template <template <typename> class TConsumer, typename TResult, typename I, typename TDelimiter>
void TestDelimiterOnRange(TResult& good, I* b, I* e, const TDelimiter& delim) {
    TResult test;
    TConsumer<TResult> consumer(&test);
    SplitString(b, e, delim, consumer);
    Cmp(good, test);
    UNIT_ASSERT_EQUAL(good, test);
}

template <typename TConsumer, typename TResult, typename I>
void TestConsumerOnString(TResult& good, I* str, I* d) {
    TResult test;
    TContainerConsumer<TResult> consumer(&test);
    TConsumer tested(&consumer);
    TCharDelimiter<const I> delim(*d);
    SplitString(str, delim, tested);
    Cmp(good, test);
    UNIT_ASSERT_EQUAL(good, test);
}

template <typename TConsumer, typename TResult, typename I>
void TestConsumerOnRange(TResult& good, I* b, I* e, I* d) {
    TResult test;
    TContainerConsumer<TResult> consumer(&test);
    TConsumer tested(&consumer);
    TCharDelimiter<const I> delim(*d);
    SplitString(b, e, delim, tested);
    Cmp(good, test);
    UNIT_ASSERT_EQUAL(good, test);
}

using TStrokaConsumer = TContainerConsumer<std::vector<std::string>>;

void TestLimitingConsumerOnString(std::vector<std::string>& good, const char* str, const char* d, size_t n, const char* last) {
    std::vector<std::string> test;
    TStrokaConsumer consumer(&test);
    TLimitingConsumer<TStrokaConsumer, const char> limits(n, &consumer);
    TCharDelimiter<const char> delim(*d);
    SplitString(str, delim, limits);
    Cmp(good, test);
    UNIT_ASSERT_EQUAL(good, test);
    UNIT_ASSERT_EQUAL(std::string(limits.Last), std::string(last)); // Quite unobvious behaviour. Why the last token is not added to slave consumer?
}

void TestLimitingConsumerOnRange(std::vector<std::string>& good, const char* b, const char* e, const char* d, size_t n, const char* last) {
    std::vector<std::string> test;
    TStrokaConsumer consumer(&test);
    TLimitingConsumer<TStrokaConsumer, const char> limits(n, &consumer);
    TCharDelimiter<const char> delim(*d);
    SplitString(b, e, delim, limits);
    Cmp(good, test);
    UNIT_ASSERT_EQUAL(good, test);
    UNIT_ASSERT_EQUAL(std::string(limits.Last), std::string(last));
}

Y_UNIT_TEST_SUITE(SplitStringTest) {
    Y_UNIT_TEST(TestCharSingleDelimiter) {
        std::string data("qw ab  qwabcab");
        std::string canonic[] = {"qw", "ab", "", "qwabcab"};
        std::vector<std::string> good(canonic, canonic + 4);
        TCharDelimiter<const char> delim(' ');

        TestDelimiterOnString<TContainerConsumer>(good, data.data(), delim);
        TestDelimiterOnRange<TContainerConsumer>(good, data.data(), data.end(), delim);
    }

    Y_UNIT_TEST(TestWideSingleDelimiter) {
        std::u16string data(u"qw ab  qwabcab");
        std::u16string canonic[] = {u"qw", u"ab", std::u16string(), u"qwabcab"};
        std::vector<std::u16string> good(canonic, canonic + 4);
        TCharDelimiter<const wchar16> delim(' ');

        TestDelimiterOnString<TContainerConsumer>(good, data.data(), delim);
        TestDelimiterOnRange<TContainerConsumer>(good, data.data(), data.end(), delim);
    }

    Y_UNIT_TEST(TestConvertToIntCharSingleDelimiter) {
        std::string data("42 4242 -12345 0");
        i32 canonic[] = {42, 4242, -12345, 0};
        std::vector<i32> good(canonic, canonic + 4);
        TCharDelimiter<const char> delim(' ');

        TestDelimiterOnString<TContainerConvertingConsumer>(good, data.data(), delim);
        TestDelimiterOnRange<TContainerConvertingConsumer>(good, data.data(), data.end(), delim);
    }

    Y_UNIT_TEST(TestCharSkipEmpty) {
        std::string data("qw ab  qwabcab ");
        std::string canonic[] = {"qw", "ab", "qwabcab"};
        std::vector<std::string> good(canonic, canonic + 3);

        TestConsumerOnString<TSkipEmptyTokens<TStrokaConsumer>>(good, data.data(), " ");
        TestConsumerOnRange<TSkipEmptyTokens<TStrokaConsumer>>(good, data.data(), data.end(), " ");
    }

    Y_UNIT_TEST(TestCharKeepDelimiters) {
        std::string data("qw ab  qwabcab ");
        std::string canonic[] = {"qw", " ", "ab", " ", "", " ", "qwabcab", " ", ""};
        std::vector<std::string> good(canonic, canonic + 9);

        TestConsumerOnString<TKeepDelimiters<TStrokaConsumer>>(good, data.data(), " ");
        TestConsumerOnRange<TKeepDelimiters<TStrokaConsumer>>(good, data.data(), data.end(), " ");
    }

    Y_UNIT_TEST(TestCharLimit) {
        std::string data("qw ab  qwabcab ");
        std::string canonic[] = {"qw", "ab"};
        std::vector<std::string> good(canonic, canonic + 2);

        TestLimitingConsumerOnString(good, data.data(), " ", 3, " qwabcab ");
        TestLimitingConsumerOnRange(good, data.data(), data.end(), " ", 3, " qwabcab ");
    }

    Y_UNIT_TEST(TestCharStringDelimiter) {
        std::string data("qw ab qwababcab");
        std::string canonic[] = {"qw ", " qw", "", "c", ""};
        std::vector<std::string> good(canonic, canonic + 5);
        TStringDelimiter<const char> delim("ab");

        TestDelimiterOnString<TContainerConsumer>(good, data.data(), delim);
        TestDelimiterOnRange<TContainerConsumer>(good, data.data(), data.end(), delim);
    }

    Y_UNIT_TEST(TestWideStringDelimiter) {
        std::u16string data(u"qw ab qwababcab");
        std::u16string canonic[] = {u"qw ", u" qw", std::u16string(), u"c", std::u16string()};
        std::vector<std::u16string> good(canonic, canonic + 5);
        std::u16string wideDelim(u"ab");
        TStringDelimiter<const wchar16> delim(wideDelim.data());

        TestDelimiterOnString<TContainerConsumer>(good, data.data(), delim);
        TestDelimiterOnRange<TContainerConsumer>(good, data.data(), data.end(), delim);
    }

    Y_UNIT_TEST(TestCharSetDelimiter) {
        std::string data("qw ab qwababccab");
        std::string canonic[] = {"q", " ab q", "abab", "", "ab"};
        std::vector<std::string> good(canonic, canonic + 5);
        TSetDelimiter<const char> delim("wc");

        TestDelimiterOnString<TContainerConsumer>(good, data.data(), delim);
        TestDelimiterOnRange<TContainerConsumer>(good, data.data(), data.end(), delim);
    }

    Y_UNIT_TEST(TestWideSetDelimiter) {
        std::u16string data(u"qw ab qwababccab");
        std::u16string canonic[] = {u"q", u" ab q", u"abab", std::u16string(), u"ab"};
        std::vector<std::u16string> good(canonic, canonic + 5);
        std::u16string wideDelim(u"wc");
        TSetDelimiter<const wchar16> delim(wideDelim.data());

        TestDelimiterOnString<TContainerConsumer>(good, data.data(), delim);
    }

    Y_UNIT_TEST(TestWideSetDelimiterRange) {
        std::u16string data(u"qw ab qwababccab");
        std::u16string canonic[] = {u"q", u" ab q", u"abab", std::u16string(), u"ab"};
        std::vector<std::u16string> good(1);
        std::u16string wideDelim(u"wc");
        TSetDelimiter<const wchar16> delim(wideDelim.data());

        std::vector<std::u16string> test;
        TContainerConsumer<std::vector<std::u16string>> consumer(&test);
        SplitString(data.data(), data.data(), delim, consumer); // Empty string is still inserted into consumer
        Cmp(good, test);

        good.assign(canonic, canonic + 4);
        good.push_back(std::u16string());
        test.clear();
        SplitString(data.data(), data.end() - 2, delim, consumer);
        Cmp(good, test);
    }

    Y_UNIT_TEST(TestSplit) {
        std::string data("qw ab qwababcba");
        std::string canonic[] = {"qw ", " qw", "c"};
        std::vector<std::string> good(canonic, canonic + 3);
        std::string delim = "ab";
        std::vector<std::string> test;
        Split(data, delim, test);
        Cmp(good, test);

        std::vector<std::string_view> test1;
        Split(data, delim.data(), test1);
        Cmp(good, test1);
    }

    Y_UNIT_TEST(ConvenientSplitTest) {
        std::string data("abc 22 33.5 xyz");
        std::string str;
        int num1 = 0;
        double num2 = 0;
        std::string_view strBuf;
        Split(data, ' ', str, num1, num2, strBuf);
        UNIT_ASSERT_VALUES_EQUAL(str, "abc");
        UNIT_ASSERT_VALUES_EQUAL(num1, 22);
        UNIT_ASSERT_VALUES_EQUAL(num2, 33.5);
        UNIT_ASSERT_VALUES_EQUAL(strBuf, "xyz");
    }

    Y_UNIT_TEST(ConvenientSplitTestWithMaybe) {
        std::string data("abc 42");
        std::string str;
        std::optional<double> num2 = 1;
        std::optional<double> maybe = 1;

        Split(data, ' ', str, num2, maybe);

        UNIT_ASSERT_VALUES_EQUAL(str, "abc");
        UNIT_ASSERT_VALUES_EQUAL(*num2, 42);
        UNIT_ASSERT(!maybe);
    }

    Y_UNIT_TEST(ConvenientSplitTestExceptions) {
        std::string data("abc 22 33");
        std::string s1, s2, s3, s4;

        UNIT_ASSERT_EXCEPTION(Split(data, ' ', s1, s2), yexception);
        UNIT_ASSERT_NO_EXCEPTION(Split(data, ' ', s1, s2, s3));
        UNIT_ASSERT_EXCEPTION(Split(data, ' ', s1, s2, s3, s4), yexception);
    }

    Y_UNIT_TEST(ConvenientSplitTestOptionalExceptions) {
        std::string data("abc 22 33");
        std::string s1, s2;
        std::optional<TString> m1, m2;

        UNIT_ASSERT_EXCEPTION(Split(data, ' ', s1, m1), yexception);
        UNIT_ASSERT_EXCEPTION(Split(data, ' ', m1, m2), yexception);
        UNIT_ASSERT_NO_EXCEPTION(Split(data, ' ', s1, s2, m1));

        UNIT_ASSERT_NO_EXCEPTION(Split(data, ' ', s1, s2, m1, m2));
        UNIT_ASSERT_EXCEPTION(Split(data, ' ', m1, m2, s1, s2), yexception);

        UNIT_ASSERT_NO_EXCEPTION(Split(data, ' ', s1, s2, m1, m2, m1, m1, m1, m1));
        UNIT_ASSERT_EXCEPTION(Split(data, ' ', s1, s2, m1, m2, m1, m1, m1, m1, s1), yexception);
    }
}

template <typename I, typename C>
void TestStringSplitterCount(I* str, C delim, size_t good) {
    auto split = StringSplitter(str).Split(delim);
    size_t res = split.Count();
    UNIT_ASSERT_VALUES_EQUAL(res, good);
    res = split.Count();
    UNIT_ASSERT_VALUES_EQUAL(res, 0);
}

Y_UNIT_TEST_SUITE(StringSplitter) {
    Y_UNIT_TEST(TestSplit) {
        int sum = 0;

        for (const auto& it : StringSplitter("1,2,3").Split(',')) {
            sum += FromString<int>(it.Token());
        }

        UNIT_ASSERT_VALUES_EQUAL(sum, 6);
    }

    Y_UNIT_TEST(TestSplit1) {
        int cnt = 0;

        for (const auto& it : StringSplitter(" ").Split(' ')) {
            (void)it;

            ++cnt;
        }

        UNIT_ASSERT_VALUES_EQUAL(cnt, 2);
    }

    Y_UNIT_TEST(TestSplitLimited) {
        std::vector<std::string> expected = {"1", "2", "3,4,5"};
        std::vector<std::string> actual = StringSplitter("1,2,3,4,5").Split(',').Limit(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSplitLimitedWithEmptySkip) {
        std::vector<std::string> expected = {"1", "2", "3,4,5"};
        std::vector<std::string> actual = StringSplitter("1,,,2,,,,3,4,5").Split(',').SkipEmpty().Limit(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);

        expected = {"1", "2", ",,,3,4,5"};
        actual = StringSplitter("1,2,,,,3,4,5").Split(',').Limit(3).SkipEmpty().ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSplitBySet) {
        int sum = 0;

        for (const auto& it : StringSplitter("1,2:3").SplitBySet(",:")) {
            sum += FromString<int>(it.Token());
        }

        UNIT_ASSERT_VALUES_EQUAL(sum, 6);
    }

    Y_UNIT_TEST(TestSplitBySetLimited) {
        std::vector<std::string> expected = {"1", "2", "3,4:5"};
        std::vector<std::string> actual = StringSplitter("1,2:3,4:5").SplitBySet(",:").Limit(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSplitBySetLimitedWithEmptySkip) {
        std::vector<std::string> expected = {"1", "2", "3,4:5"};
        std::vector<std::string> actual = StringSplitter("1,:,2::::,3,4:5").SplitBySet(",:").SkipEmpty().Limit(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);

        expected = {"1", ",2::::,3,4:5"};
        actual = StringSplitter("1,:,2::::,3,4:5").SplitBySet(",:").Limit(3).SkipEmpty().ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSplitByString) {
        int sum = 0;

        for (const auto& it : StringSplitter("1ab2ab3").SplitByString("ab")) {
            sum += FromString<int>(it.Token());
        }

        UNIT_ASSERT_VALUES_EQUAL(sum, 6);
    }

    Y_UNIT_TEST(TestSplitByStringLimited) {
        std::vector<std::string> expected = {"1", "2", "3ab4ab5"};
        std::vector<std::string> actual = StringSplitter("1ab2ab3ab4ab5").SplitByString("ab").Limit(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSplitByStringLimitedWithEmptySkip) {
        std::vector<std::string> expected = {"1", "2", "3ab4ab5"};
        std::vector<std::string> actual = StringSplitter("1abab2ababababab3ab4ab5").SplitByString("ab").SkipEmpty().Limit(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSplitByFunc) {
        std::string s = "123 456 \t\n789\n10\t 20";
        std::vector<std::string> pattern = {"123", "456", "789", "10", "20"};

        std::vector<std::string> tokens;
        auto f = [](char a) { return a == ' ' || a == '\t' || a == '\n'; };
        for (auto v : StringSplitter(s).SplitByFunc(f)) {
            if (v) {
                tokens.emplace_back(v);
            }
        }

        UNIT_ASSERT(tokens == pattern);
    }

    Y_UNIT_TEST(TestSplitByFuncLimited) {
        std::vector<std::string> expected = {"1", "2", "3a4b5"};
        auto f = [](char a) { return a == 'a' || a == 'b'; };
        std::vector<std::string> actual = StringSplitter("1a2b3a4b5").SplitByFunc(f).Limit(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSplitByFuncLimitedWithEmptySkip) {
        std::vector<std::string> expected = {"1", "2", "3a4b5"};
        auto f = [](char a) { return a == 'a' || a == 'b'; };
        std::vector<std::string> actual = StringSplitter("1aaba2bbababa3a4b5").SplitByFunc(f).SkipEmpty().Limit(3).Take(3).ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestSkipEmpty) {
        int sum = 0;

        for (const auto& it : StringSplitter("  1 2 3   ").Split(' ').SkipEmpty()) {
            sum += FromString<int>(it.Token());
        }

        UNIT_ASSERT_VALUES_EQUAL(sum, 6);

        // double
        sum = 0;
        for (const auto& it : StringSplitter("  1 2 3   ").Split(' ').SkipEmpty().SkipEmpty()) {
            sum += FromString<int>(it.Token());
        }
        UNIT_ASSERT_VALUES_EQUAL(sum, 6);
    }

    Y_UNIT_TEST(TestTake) {
        std::vector<std::string> expected = {"1", "2", "3"};
        UNIT_ASSERT_VALUES_EQUAL(expected, StringSplitter("1 2 3 4 5 6 7 8 9 10").Split(' ').Take(3).ToList<std::string>());

        expected = {"1", "2"};
        UNIT_ASSERT_VALUES_EQUAL(expected, StringSplitter("  1 2 3   ").Split(' ').SkipEmpty().Take(2).ToList<std::string>());

        expected = {"1", "2", "3"};
        UNIT_ASSERT_VALUES_EQUAL(expected, StringSplitter("1 2 3 4 5 6 7 8 9 10").Split(' ').Take(5).Take(3).ToList<std::string>());
        UNIT_ASSERT_VALUES_EQUAL(expected, StringSplitter("1 2 3 4 5 6 7 8 9 10").Split(' ').Take(3).Take(5).ToList<std::string>());

        expected = {"1", "2"};
        UNIT_ASSERT_VALUES_EQUAL(expected, StringSplitter("  1 2 3  ").Split(' ').Take(4).SkipEmpty().ToList<std::string>());

        expected = {"1"};
        UNIT_ASSERT_VALUES_EQUAL(expected, StringSplitter("  1 2 3  ").Split(' ').Take(4).SkipEmpty().Take(1).ToList<std::string>());
    }

    Y_UNIT_TEST(TestCompile) {
        (void)StringSplitter(std::string());
        (void)StringSplitter(std::string_view());
        (void)StringSplitter("", 0);
    }

    Y_UNIT_TEST(TestStringSplitterCountEmpty) {
        TCharDelimiter<const char> delim(' ');
        TestStringSplitterCount("", delim, 1);
    }

    Y_UNIT_TEST(TestStringSplitterCountOne) {
        TCharDelimiter<const char> delim(' ');
        TestStringSplitterCount("one", delim, 1);
    }

    Y_UNIT_TEST(TestStringSplitterCountWithOneDelimiter) {
        TCharDelimiter<const char> delim(' ');
        TestStringSplitterCount("one two", delim, 2);
    }

    Y_UNIT_TEST(TestStringSplitterCountWithTrailing) {
        TCharDelimiter<const char> delim(' ');
        TestStringSplitterCount(" one ", delim, 3);
    }

    Y_UNIT_TEST(TestStringSplitterConsume) {
        std::vector<std::string> expected = {"1", "2", "3"};
        std::vector<std::string> actual;
        auto func = [&actual](const std::basic_string_view<char>& token) {
            actual.push_back(std::string(token));
        };
        StringSplitter("1 2 3").Split(' ').Consume(func);
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestStringSplitterConsumeConditional) {
        std::vector<std::string> expected = {"1", "2"};
        std::vector<std::string> actual;
        auto func = [&actual](const std::basic_string_view<char>& token) {
            if (token == "3") {
                return false;
            }
            actual.push_back(std::string(token));
            return true;
        };
        bool completed = StringSplitter("1 2 3 4 5").Split(' ').Consume(func);
        UNIT_ASSERT(!completed);
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestStringSplitterToList) {
        std::vector<std::string> expected = {"1", "2", "3"};
        std::vector<std::string> actual = StringSplitter("1 2 3").Split(' ').ToList<std::string>();
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestStringSplitterCollectPushBack) {
        std::vector<std::string> expected = {"1", "2", "3"};
        std::vector<std::string> actual;
        StringSplitter("1 2 3").Split(' ').Collect(&actual);
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestStringSplitterCollectInsert) {
        TSet<std::string> expected = {"1", "2", "3"};
        TSet<std::string> actual;
        StringSplitter("1 2 3 1 2 3").Split(' ').Collect(&actual);
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestStringSplitterCollectClears) {
        std::vector<std::string> v;
        StringSplitter("1 2 3").Split(' ').Collect(&v);
        UNIT_ASSERT_VALUES_EQUAL(v.size(), 3);
        StringSplitter("4 5").Split(' ').Collect(&v);
        UNIT_ASSERT_VALUES_EQUAL(v.size(), 2);
    }

    Y_UNIT_TEST(TestStringSplitterAddToDoesntClear) {
        std::vector<std::string> v;
        StringSplitter("1 2 3").Split(' ').AddTo(&v);
        UNIT_ASSERT_VALUES_EQUAL(v.size(), 3);
        StringSplitter("4 5").Split(' ').AddTo(&v);
        UNIT_ASSERT_VALUES_EQUAL(v.size(), 5);
    }

    Y_UNIT_TEST(TestSplitStringInto) {
        int a = -1;
        std::string_view s;
        double d = -1;
        StringSplitter("2 substr 1.02").Split(' ').CollectInto(&a, &s, &d);
        UNIT_ASSERT_VALUES_EQUAL(a, 2);
        UNIT_ASSERT_VALUES_EQUAL(s, "substr");
        UNIT_ASSERT_DOUBLES_EQUAL(d, 1.02, 0.0001);
        UNIT_ASSERT_EXCEPTION(StringSplitter("1").Split(' ').CollectInto(&a, &a), yexception);
        UNIT_ASSERT_EXCEPTION(StringSplitter("1 2 3").Split(' ').CollectInto(&a, &a), yexception);
    }

    Y_UNIT_TEST(TestSplitStringWithIgnore) {
        std::string_view s;
        StringSplitter("x y z").Split(' ').CollectInto(&std::ignore, &s, &std::ignore);
        UNIT_ASSERT_VALUES_EQUAL(s, "y");

        UNIT_ASSERT_EXCEPTION(StringSplitter("ignored != non-requred").Split(':').CollectInto(&s, &std::ignore), yexception);
    }

    Y_UNIT_TEST(TestTryCollectInto) {
        int a, b, c;
        bool parsingSucceeded;
        parsingSucceeded = StringSplitter("100,500,3").Split(',').TryCollectInto(&a, &b, &c);
        UNIT_ASSERT(parsingSucceeded);
        UNIT_ASSERT_VALUES_EQUAL(a, 100);
        UNIT_ASSERT_VALUES_EQUAL(b, 500);
        UNIT_ASSERT_VALUES_EQUAL(c, 3);

        //not enough tokens
        parsingSucceeded = StringSplitter("3,14").Split(',').TryCollectInto(&a, &b, &c);
        UNIT_ASSERT(!parsingSucceeded);

        //too many tokens
        parsingSucceeded = StringSplitter("3,14,15,92,6").Split(',').TryCollectInto(&a, &b, &c);
        UNIT_ASSERT(!parsingSucceeded);

        //where single TryFromString fails
        parsingSucceeded = StringSplitter("ot topota kopyt pyl po polu letit").Split(' ').TryCollectInto(&a, &b, &c);
        UNIT_ASSERT(!parsingSucceeded);
    }

    Y_UNIT_TEST(TestOwningSplit1) {
        int sum = 0;

        for (const auto& it : StringSplitter(std::string("1,2,3")).Split(',')) {
            sum += FromString<int>(it.Token());
        }

        UNIT_ASSERT_VALUES_EQUAL(sum, 6);
    }

    Y_UNIT_TEST(TestOwningSplit2) {
        int sum = 0;

        std::string str("1,2,3");
        for (const auto& it : StringSplitter(str).Split(',')) {
            sum += FromString<int>(it.Token());
        }

        UNIT_ASSERT_VALUES_EQUAL(sum, 6);
    }

    Y_UNIT_TEST(TestOwningSplit3) {
        int sum = 0;

        const std::string str("1,2,3");
        for (const auto& it : StringSplitter(str).Split(',')) {
            sum += FromString<int>(it.Token());
        }

        UNIT_ASSERT_VALUES_EQUAL(sum, 6);
    }

    Y_UNIT_TEST(TestAssigment) {
        std::vector<std::string> expected0 = {"1", "2", "3", "4"};
        std::vector<std::string> actual0 = StringSplitter("1 2 3 4").Split(' ');
        UNIT_ASSERT_VALUES_EQUAL(expected0, actual0);

        TSet<std::string> expected1 = {"11", "22", "33", "44"};
        TSet<std::string> actual1 = StringSplitter("11 22 33 44").Split(' ');
        UNIT_ASSERT_VALUES_EQUAL(expected1, actual1);

        TSet<std::string> expected2 = {"11", "aa"};
        auto actual2 = static_cast<TSet<std::string>>(StringSplitter("11 aa 11 11 aa").Split(' '));
        UNIT_ASSERT_VALUES_EQUAL(expected2, actual2);

        std::vector<std::string> expected3 = {"dd", "bb"};
        auto actual3 = std::vector<std::string>(StringSplitter("dd\tbb").Split('\t'));
        UNIT_ASSERT_VALUES_EQUAL(expected3, actual3);
    }

    Y_UNIT_TEST(TestRangeBasedFor) {
        std::vector<std::string> actual0 = {"11", "22", "33", "44"};
        size_t num = 0;
        for (std::string_view elem : StringSplitter("11 22 33 44").Split(' ')) {
            UNIT_ASSERT_VALUES_EQUAL(elem, actual0[num++]);
        }

        std::vector<std::string> actual1 = {"another", "one,", "and", "another", "one"};
        num = 0;
        for (std::string_view elem : StringSplitter(std::string_view("another one, and \n\n     another    one")).SplitBySet(" \n").SkipEmpty()) {
            UNIT_ASSERT_VALUES_EQUAL(elem, actual1[num++]);
        }

        std::vector<std::u16string> actual2 = {u"привет,", u"как", u"дела"};
        num = 0;
        for (std::u16string_view elem : StringSplitter(u"привет, как дела").Split(wchar16(' '))) {
            UNIT_ASSERT_VALUES_EQUAL(elem, actual2[num++]);
        }

        std::vector<std::string> copy(4);
        auto v = StringSplitter("11 22 33 44").Split(' ');
        Copy(v.begin(), v.end(), copy.begin());
        UNIT_ASSERT_VALUES_EQUAL(actual0, copy);
    }

    Y_UNIT_TEST(TestParseInto) {
        std::vector<int> actual0 = {1, 2, 3, 4};
        std::vector<int> answer0;

        StringSplitter("1 2 3 4").Split(' ').ParseInto(&answer0);
        UNIT_ASSERT_VALUES_EQUAL(actual0, answer0);

        std::vector<int> actual1 = {42, 1, 2, 3, 4};
        std::vector<int> answer1 = {42};
        StringSplitter("1 2 3 4").Split(' ').ParseInto(&answer1);
        UNIT_ASSERT_VALUES_EQUAL(actual1, answer1);

        answer1.clear();
        UNIT_ASSERT_EXCEPTION(StringSplitter("1 2    3 4").Split(' ').ParseInto(&answer1), yexception);

        answer1 = {42};
        StringSplitter("   1    2     3 4").Split(' ').SkipEmpty().ParseInto(&answer1);
        UNIT_ASSERT_VALUES_EQUAL(actual1, answer1);

        answer1.clear();
        StringSplitter("  \n 1    2  \n\n\n   3 4\n ").SplitBySet(" \n").SkipEmpty().ParseInto(&answer1);
        UNIT_ASSERT_VALUES_EQUAL(actual0, answer1);
    }

    Y_UNIT_TEST(TestStdString) {
        std::vector<std::string_view> r0, r1, answer = {"lol", "zomg"};
        std::string s = "lol zomg";
        for (std::string_view ss : StringSplitter(s).Split(' ')) {
            r0.push_back(ss);
        }
        StringSplitter(s).Split(' ').Collect(&r1);

        UNIT_ASSERT_VALUES_EQUAL(r0, answer);
        UNIT_ASSERT_VALUES_EQUAL(r1, answer);
    }

    Y_UNIT_TEST(TestStdStringView) {
        std::string_view s = "aaacccbbb";
        std::vector<std::string_view> expected = {"aaa", "bbb"};
        std::vector<std::string_view> actual = StringSplitter(s).SplitByString("ccc");
        UNIT_ASSERT_VALUES_EQUAL(expected, actual);
    }

    Y_UNIT_TEST(TestStdSplitAfterSplit) {
        std::string_view input = "a*b+a*b";
        for (std::string_view summand : StringSplitter(input).Split('+')) {
            //FIXME: std::string is used to workaround MSVC ICE
            UNIT_ASSERT_VALUES_EQUAL(std::string(summand), "a*b");
            std::string_view multiplier1, multiplier2;
            bool splitResult = StringSplitter(summand).Split('*').TryCollectInto(&multiplier1, &multiplier2);
            UNIT_ASSERT(splitResult);
            UNIT_ASSERT_VALUES_EQUAL(std::string(multiplier1), "a");
            UNIT_ASSERT_VALUES_EQUAL(std::string(multiplier2), "b");
        }
    }

    Y_UNIT_TEST(TestStdSplitWithParsing) {
        std::string_view input = "1,2,3,4";
        std::vector<ui64> numbers;
        const std::vector<ui64> expected{1, 2, 3, 4};
        StringSplitter(input).Split(',').ParseInto(&numbers);
        UNIT_ASSERT_VALUES_EQUAL(numbers, expected);
    }

    Y_UNIT_TEST(TestArcadiaStdInterop) {
        std::vector<std::string> expected0 = {"a", "b"};
        std::vector<std::string_view> expected1 = {"a", "b"};
        std::string src1("a  b");
        std::string_view src2("a  b");
        std::vector<std::string> actual0 = StringSplitter(src1).Split(' ').SkipEmpty();
        std::vector<std::string> actual1 = StringSplitter(src2).Split(' ').SkipEmpty();
        std::vector<std::string_view> actual2 = StringSplitter(src1).Split(' ').SkipEmpty();
        std::vector<std::string_view> actual3 = StringSplitter(src2).Split(' ').SkipEmpty();
        UNIT_ASSERT_VALUES_EQUAL(expected0, actual0);
        UNIT_ASSERT_VALUES_EQUAL(expected0, actual1);
        UNIT_ASSERT_VALUES_EQUAL(expected1, actual2);
        UNIT_ASSERT_VALUES_EQUAL(expected1, actual3);
    }

    Y_UNIT_TEST(TestConstCString) {
        const char* b = "a;b";
        const char* e = b + 3;

        std::vector<std::string_view> v;
        StringSplitter(b, e).Split(';').AddTo(&v);

        std::vector<std::string_view> expected = {"a", "b"};
        UNIT_ASSERT_VALUES_EQUAL(v, expected);
    }

    Y_UNIT_TEST(TestCStringRef) {
        std::string s = "lol";
        char* str = s.Detach();

        std::vector<std::string_view> v = StringSplitter(str).Split('o');
        std::vector<std::string_view> expected = {"l", "l"};
        UNIT_ASSERT_VALUES_EQUAL(v, expected);
    }

    Y_UNIT_TEST(TestSplitVector) {
        std::vector<char> buffer = {'a', ';', 'b'};

        std::vector<std::string_view> v = StringSplitter(buffer).Split(';');

        std::vector<std::string_view> expected = {"a", "b"};
        UNIT_ASSERT_VALUES_EQUAL(v, expected);
    }

    class TDoubleIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = int;
        using pointer = void;
        using reference = int;
        using const_reference = int;
        using difference_type = ptrdiff_t;

        TDoubleIterator() = default;

        TDoubleIterator(const char* ptr)
            : Ptr_(ptr)
        {
        }

        TDoubleIterator operator++() {
            Ptr_ += 2;
            return *this;
        }

        TDoubleIterator operator++(int) {
            TDoubleIterator tmp = *this;
            ++*this;
            return tmp;
        }

        friend bool operator==(TDoubleIterator l, TDoubleIterator r) {
            return l.Ptr_ == r.Ptr_;
        }

        friend bool operator!=(TDoubleIterator l, TDoubleIterator r) {
            return l.Ptr_ != r.Ptr_;
        }

        int operator*() const {
            return (*Ptr_ - '0') * 10 + *(Ptr_ + 1) - '0';
        }

    private:
        const char* Ptr_ = nullptr;
    };

    Y_UNIT_TEST(TestInputIterator) {
        const char* beg = "1213002233000011";
        const char* end = beg + strlen(beg);

        std::vector<std::vector<int>> expected = {{12, 13}, {22, 33}, {}, {11}};
        int i = 0;

        for (TIteratorRange<TDoubleIterator> part : StringSplitter(TDoubleIterator(beg), TDoubleIterator(end)).SplitByFunc([](int value) { return value == 0; })) {
            UNIT_ASSERT(std::equal(part.begin(), part.end(), expected[i].begin(), expected[i].end()));
            i++;
        }
        UNIT_ASSERT_VALUES_EQUAL(i, expected.size());
    }
}
