#include "iterator.h"

#include <library/cpp/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TIterator) {
    Y_UNIT_TEST(ToForwardIteratorTest) {
        std::vector<int> x = {1, 2};
        UNIT_ASSERT_VALUES_EQUAL(*std::prev(x.end()), *ToForwardIterator(x.rbegin()));
        UNIT_ASSERT_VALUES_EQUAL(*ToForwardIterator(std::prev(x.rend())), *x.begin());
    }
}

Y_UNIT_TEST_SUITE(TInputRangeAdaptor) {
    class TSquaresGenerator: public TInputRangeAdaptor<TSquaresGenerator> {
    public:
        const i64* Next() {
            Current_ = State_ * State_;
            ++State_;
            // Never return nullptr => we have infinite range!
            return &Current_;
        }

    private:
        i64 State_ = 0.0;
        i64 Current_ = 0.0;
    };

    Y_UNIT_TEST(TSquaresGenerator) {
        i64 cur = 0;
        for (i64 sqr : TSquaresGenerator{}) {
            UNIT_ASSERT_VALUES_EQUAL(cur * cur, sqr);

            if (++cur > 10) {
                break;
            }
        }
    }

    class TUrlPart: public TInputRangeAdaptor<TUrlPart> {
    public:
        TUrlPart(const std::string_view& url)
            : Url_(url)
        {
        }

        NStlIterator::TProxy<std::string_view> Next() {
            return Url_.NextTok('/');
        }

    private:
        std::string_view Url_;
    };

    Y_UNIT_TEST(TUrlPart) {
        const std::vector<std::string_view> expected = {std::string_view("yandex.ru"), std::string_view("search?")};
        auto expected_part = expected.begin();
        for (const std::string_view& part : TUrlPart(std::string_view("yandex.ru/search?"))) {
            UNIT_ASSERT_VALUES_EQUAL(part, *expected_part);
            ++expected_part;
        }
        UNIT_ASSERT(expected_part == expected.end());
    }
}
