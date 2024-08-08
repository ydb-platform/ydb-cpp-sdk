#include "common_ops.h"
#include "random.h"

#include <library/cpp/testing/unittest/registar.h>

#include <util/digest/numeric.h>

#include <random>

Y_UNIT_TEST_SUITE(TestCommonRNG) {
    template <class T>
    struct TRng: public TCommonRNG<T, TRng<T>> {
        inline T GenRand() noexcept {
            return IntHash(C_++);
        }

        T C_ = RandomNumber<T>();
    };

    Y_UNIT_TEST(TestUniform1) {
        TRng<ui32> r;

        for (size_t i = 0; i < 1000; ++i) {
            UNIT_ASSERT(r.Uniform(10) < 10);
        }
    }

    Y_UNIT_TEST(TestUniform2) {
        TRng<ui32> r;

        for (size_t i = 0; i < 1000; ++i) {
            UNIT_ASSERT(r.Uniform(1) == 0);
        }
    }

    Y_UNIT_TEST(TestUniform3) {
        TRng<ui32> r;

        for (size_t i = 0; i < 1000; ++i) {
            auto x = r.Uniform(100, 200);

            UNIT_ASSERT(x >= 100);
            UNIT_ASSERT(x < 200);
        }
    }

    Y_UNIT_TEST(TestStlCompatibility) {
        // NOTE: Check if `TRng` can be passed to `std::normal_distribution::operator()`.
        // These tests just have to be compilable, so the asserts below are always true.
        {
            TRng<ui32> r;
            std::normal_distribution<float> nd(0, 1);
            UNIT_ASSERT_DOUBLES_EQUAL(nd(r), 0.0, nd.max());
        }

        {
            TRng<ui64> r;
            std::normal_distribution<double> nd(0, 1);
            UNIT_ASSERT_DOUBLES_EQUAL(nd(r), 0.0, nd.max());
        }

        {
            TRng<ui16> r;
            std::normal_distribution<long double> nd(0, 1);
            UNIT_ASSERT_DOUBLES_EQUAL(nd(r), 0.0, nd.max());
        }
    }
}
