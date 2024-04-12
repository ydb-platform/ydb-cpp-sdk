#include "golovan_page.h"

#include <src/library/monlib/service/pages/templates.h>

#include <src/util/string/split.h>
#include <src/util/system/tls.h>

using namespace NMonitoring;

class TGolovanCountableConsumer: public ICountableConsumer {
public:
    using TOutputCallback = std::function<void()>;

    TGolovanCountableConsumer(IOutputStream& out, TOutputCallback& OutputCallback)
        : out(out)
    {
        if (OutputCallback) {
            OutputCallback();
        }

        out << HTTPOKJSON << "[";
        FirstCounter = true;
    }

    void OnCounter(const std::string&, const std::string& value, const TCounterForPtr* counter) override {
        if (FirstCounter) {
            FirstCounter = false;
        } else {
            out << ",";
        }

        out << "[\"" << prefix + value;
        if (counter->ForDerivative()) {
            out << "_dmmm";
        } else {
            out << "_ahhh";
        }

        out << "\"," << counter->Val() << "]";
    }

    void OnHistogram(const std::string&, const std::string&, IHistogramSnapshotPtr, bool) override {
    }

    void OnGroupBegin(const std::string&, const std::string& value, const TDynamicCounters*) override {
        prefix += value;
        if (!value.empty()) {
            prefix += "_";
        }
    }

    void OnGroupEnd(const std::string&, const std::string&, const TDynamicCounters*) override {
        prefix = "";
    }

    void Flush() {
        out << "]";
        out.Flush();
    }

private:
    IOutputStream& out;
    bool FirstCounter;
    std::string prefix;
};

TGolovanCountersPage::TGolovanCountersPage(const std::string& path, TIntrusivePtr<NMonitoring::TDynamicCounters> counters,
                                           TOutputCallback outputCallback)
    : IMonPage(path)
    , Counters(counters)
    , OutputCallback(outputCallback)
{
}

void TGolovanCountersPage::Output(IMonHttpRequest& request) {
    TGolovanCountableConsumer consumer(request.Output(), OutputCallback);
    Counters->Accept("", "", consumer);
    consumer.Flush();
}
