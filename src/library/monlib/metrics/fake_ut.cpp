#include "fake.h"

#include <src/library/testing/unittest/registar.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

using namespace NMonitoring;

Y_UNIT_TEST_SUITE(TFakeTest) {

    Y_UNIT_TEST(CreateOnStack) {
        TFakeMetricRegistry registry;
    }

    Y_UNIT_TEST(CreateOnHeap) {
        auto registry = MakeAtomicShared<TFakeMetricRegistry>();
        UNIT_ASSERT(registry);
    }

    Y_UNIT_TEST(Gauge) {
        TFakeMetricRegistry registry(TLabels{{"common", "label"}});

        IGauge* g = registry.Gauge(MakeLabels({{"my", "gauge"}}));
        UNIT_ASSERT(g);

        UNIT_ASSERT_DOUBLES_EQUAL(g->Get(), 0.0, 1E-6);
        g->Set(12.34);
        UNIT_ASSERT_DOUBLES_EQUAL(g->Get(), 0.0, 1E-6); // no changes

        double val = g->Add(1.2);
        UNIT_ASSERT_DOUBLES_EQUAL(g->Get(), 0.0, 1E-6);
        UNIT_ASSERT_DOUBLES_EQUAL(val, 0.0, 1E-6);
    }
}
