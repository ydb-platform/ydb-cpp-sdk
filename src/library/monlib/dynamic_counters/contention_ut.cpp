#include "counters.h"
#include <src/library/testing/unittest/registar.h>
#include <src/util/system/event.h>
#include <src/util/system/thread.h>

using namespace NMonitoring;

Y_UNIT_TEST_SUITE(TDynamicCountersContentionTest) {

    Y_UNIT_TEST(EnsureNonlocking) {
        TDynamicCounterPtr counters = MakeIntrusive<TDynamicCounters>();

        class TConsumer : public ICountableConsumer {
            TAutoEvent Ev;
            TAutoEvent Response;
            TDynamicCounterPtr Counters;
            TThread Thread;

        public:
            TConsumer(TDynamicCounterPtr counters)
                : Counters(counters)
                , Thread(std::bind(&TConsumer::ThreadFunc, this))
            {
                Thread.Start();
            }

            ~TConsumer() override {
                Thread.Join();
            }

            void OnCounter(const std::string& /*labelName*/, const std::string& /*labelValue*/, const TCounterForPtr* /*counter*/) override {
                Ev.Signal();
                Response.Wait();
            }

            void OnHistogram(const std::string& /*labelName*/, const std::string& /*labelValue*/, IHistogramSnapshotPtr /*snapshot*/, bool /*derivative*/) override {
            }

            void OnGroupBegin(const std::string& /*labelName*/, const std::string& /*labelValue*/, const TDynamicCounters* /*group*/) override {
            }

            void OnGroupEnd(const std::string& /*labelName*/, const std::string& /*labelValue*/, const TDynamicCounters* /*group*/) override {
            }

        private:
            void ThreadFunc() {
                // acts like a coroutine
                Ev.Wait();
                auto ctr = Counters->GetSubgroup("label", "value")->GetCounter("name");
                Y_ABORT_UNLESS(*ctr == 42);
                Response.Signal();
            }
        };

        auto ctr = counters->GetSubgroup("label", "value")->GetCounter("name");
        *ctr = 42;
        TConsumer consumer(counters);
        counters->Accept({}, {}, consumer);
    }

}
