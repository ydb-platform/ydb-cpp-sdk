#include "adaptor.h"
#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <src/library/testing/unittest/registar.h>

struct TOnCopy: yexception {
};

struct TOnMove: yexception {
};

struct TState {
    explicit TState() {
    }

    TState(const TState&) {
        ythrow TOnCopy();
    }

    TState(TState&&) {
        ythrow TOnMove();
    }

    void operator=(const TState&) {
        ythrow TOnCopy();
    }

    void rbegin() const {
    }

    void rend() const {
    }
};

Y_UNIT_TEST_SUITE(TReverseAdaptor) {
    Y_UNIT_TEST(ReadTest) {
        std::vector<int> cont = {1, 2, 3};
        std::vector<int> etalon = {3, 2, 1};
        size_t idx = 0;
        for (const auto& x : Reversed(cont)) {
            UNIT_ASSERT_VALUES_EQUAL(etalon[idx++], x);
        }
        idx = 0;
        for (const auto& x : Reversed(std::move(cont))) {
            UNIT_ASSERT_VALUES_EQUAL(etalon[idx++], x);
        }
    }

    Y_UNIT_TEST(WriteTest) {
        std::vector<int> cont = {1, 2, 3};
        std::vector<int> etalon = {3, 6, 9};
        size_t idx = 0;
        for (auto& x : Reversed(cont)) {
            x *= x + idx++;
        }
        idx = 0;
        for (auto& x : cont) {
            UNIT_ASSERT_VALUES_EQUAL(etalon[idx++], x);
        }
    }

    Y_UNIT_TEST(InnerTypeTest) {
        using TStub = std::vector<int>;
        TStub stub;
        const TStub cstub;

        using namespace NPrivate;
        UNIT_ASSERT_TYPES_EQUAL(decltype(Reversed(stub)), TReverseRange<TStub&>);
        UNIT_ASSERT_TYPES_EQUAL(decltype(Reversed(cstub)), TReverseRange<const TStub&>);
    }

    Y_UNIT_TEST(CopyMoveTest) {
        TState lvalue;
        const TState clvalue;
        UNIT_ASSERT_NO_EXCEPTION(Reversed(lvalue));
        UNIT_ASSERT_NO_EXCEPTION(Reversed(clvalue));
    }

    Y_UNIT_TEST(ReverseX2Test) {
        std::vector<int> cont = {1, 2, 3};
        size_t idx = 0;
        for (const auto& x : Reversed(Reversed(cont))) {
            UNIT_ASSERT_VALUES_EQUAL(cont[idx++], x);
        }
    }

    Y_UNIT_TEST(ReverseX3Test) {
        std::vector<int> cont = {1, 2, 3};
        std::vector<int> etalon = {3, 2, 1};
        size_t idx = 0;
        for (const auto& x : Reversed(Reversed(Reversed(cont)))) {
            UNIT_ASSERT_VALUES_EQUAL(etalon[idx++], x);
        }
    }

    Y_UNIT_TEST(ReverseTemporaryTest) {
        std::vector<int> etalon = {3, 2, 1};
        std::vector<int> etalon2 = {1, 2, 3};
        size_t idx = 0;
        for (const auto& x : Reversed(std::vector<int>{1, 2, 3})) {
            UNIT_ASSERT_VALUES_EQUAL(etalon[idx++], x);
        }
        idx = 0;
        for (const auto& x : Reversed(Reversed(std::vector<int>{1, 2, 3}))) {
            UNIT_ASSERT_VALUES_EQUAL(etalon2[idx++], x);
        }
    }

    Y_UNIT_TEST(ReverseInitializerListTest) {
        // initializer_list has no rbegin and rend
        auto cont = {1, 2, 3};
        std::vector<int> etalon = {3, 2, 1};
        std::vector<int> etalon2 = {1, 2, 3};

        size_t idx = 0;
        for (const auto& x : Reversed(cont)) {
            UNIT_ASSERT_VALUES_EQUAL(etalon[idx++], x);
        }
        idx = 0;
        for (const auto& x : Reversed(Reversed(cont))) {
            UNIT_ASSERT_VALUES_EQUAL(etalon2[idx++], x);
        }
    }
}
