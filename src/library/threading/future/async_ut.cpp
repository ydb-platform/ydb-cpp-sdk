#include "async.h"

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


namespace {
    struct TMySuperTaskQueue {
    };

}

namespace NThreading {
    /* Here we provide an Async overload for TMySuperTaskQueue indide NThreading namespace
 * so that we can call it in the way
 *
 * TMySuperTaskQueue queue;
 * NThreading::Async([](){}, queue);
 *
 * See also ExtensionExample unittest.
 */
    template <typename Func>
    TFuture<TFunctionResult<Func>> Async(Func func, TMySuperTaskQueue&) {
        return MakeFuture(func());
    }

}

Y_UNIT_TEST_SUITE(Async) {
    Y_UNIT_TEST(ExtensionExample) {
        TMySuperTaskQueue queue;
        auto future = NThreading::Async([]() { return 5; }, queue);
        future.Wait();
        UNIT_ASSERT_VALUES_EQUAL(future.GetValue(), 5);
    }

    Y_UNIT_TEST(WorksWithIMtpQueue) {
        auto queue = MakeHolder<TThreadPool>();
        queue->Start(1);

        auto future = NThreading::Async([]() { return 5; }, *queue);
        future.Wait();
        UNIT_ASSERT_VALUES_EQUAL(future.GetValue(), 5);
    }

    Y_UNIT_TEST(ProperlyDeducesFutureType) {
        // Compileability test
        auto queue = CreateThreadPool(1);

        NThreading::TFuture<void> f1 = NThreading::Async([]() {}, *queue);
        NThreading::TFuture<int> f2 = NThreading::Async([]() { return 5; }, *queue);
        NThreading::TFuture<double> f3 = NThreading::Async([]() { return 5.0; }, *queue);
        NThreading::TFuture<std::vector<int>> f4 = NThreading::Async([]() { return std::vector<int>(); }, *queue);
        NThreading::TFuture<int> f5 = NThreading::Async([]() { return NThreading::MakeFuture(5); }, *queue);
    }
}
