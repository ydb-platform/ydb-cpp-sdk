#include "registar.h"

#include <ydb-cpp-sdk/util/datetime/base.h>
#include <src/util/system/tempfile.h>

#include <optional>

namespace NUnitTest {

extern const std::string Y_UNITTEST_OUTPUT_CMDLINE_OPTION;
extern const std::string Y_UNITTEST_TEST_FILTER_FILE_OPTION;

class TJUnitProcessor : public ITestSuiteProcessor {
    struct TFailure {
        std::string Message;
        std::string BackTrace;
    };

    struct TTestCase {
        std::string Name;
        bool Success;
        std::vector<TFailure> Failures;
        std::string StdOut;
        std::string StdErr;
        double DurationSecods = 0.0;

        size_t GetFailuresCount() const {
            return Failures.size();
        }
    };

    struct TTestSuite {
        std::map<std::string, TTestCase> Cases;

        size_t GetTestsCount() const {
            return Cases.size();
        }

        size_t GetFailuresCount() const {
            size_t sum = 0;
            for (const auto& [name, testCase] : Cases) {
                sum += testCase.GetFailuresCount();
            }
            return sum;
        }

        double GetDurationSeconds() const {
            double sum = 0.0;
            for (const auto& [name, testCase] : Cases) {
                sum += testCase.DurationSecods;
            }
            return sum;
        }
    };

    // Holds a copy of TTest structure for current test
    class TCurrentTest {
    public:
        TCurrentTest(const TTest* test)
            : TestName(test->name)
            , Unit(*test->unit)
            , Test{&Unit, TestName.c_str()}
        {
        }

        operator const TTest*() const {
            return &Test;
        }

    private:
        std::string TestName;
        TUnit Unit;
        TTest Test;
    };

    struct TOutputCapturer;

public:
    enum class EOutputFormat {
        Xml,
        Json,
    };

    TJUnitProcessor(std::string file, std::string exec, EOutputFormat outputFormat);
    ~TJUnitProcessor();

    void SetForkTestsParams(bool forkTests, bool isForked) override;

    void OnBeforeTest(const TTest* test) override;
    void OnError(const TError* descr) override;
    void OnFinish(const TFinish* descr) override;

private:
    TTestCase* GetTestCase(const TTest* test) {
        auto& suite = Suites[test->unit->name];
        return &suite.Cases[test->name];
    }

    void Save();

    size_t GetTestsCount() const {
        size_t sum = 0;
        for (const auto& [name, suite] : Suites) {
            sum += suite.GetTestsCount();
        }
        return sum;
    }

    size_t GetFailuresCount() const {
        size_t sum = 0;
        for (const auto& [name, suite] : Suites) {
            sum += suite.GetFailuresCount();
        }
        return sum;
    }

    void SerializeToFile();
    void SerializeToXml();
    void SerializeToJson();
    void MergeSubprocessReport();

    std::string BuildFileName(size_t index, const std::string_view extension) const;
    std::string_view GetFileExtension() const;
    void MakeReportFileName();
    void MakeTmpFileNameForForkedTests();
    static void TransferFromCapturer(std::unique_ptr<TJUnitProcessor::TOutputCapturer>& capturer, std::string& out, std::ostream& outStream);

    static void CaptureSignal(TJUnitProcessor* processor);
    static void UncaptureSignal();
    static void SignalHandler(int signal);

private:
    const std::string FileName; // cmd line param
    const std::string ExecName; // cmd line param
    const EOutputFormat OutputFormat;
    std::string ResultReportFileName;
    std::optional<TTempFile> TmpReportFile;
    std::map<std::string, TTestSuite> Suites;
    std::unique_ptr<TOutputCapturer> StdErrCapturer;
    std::unique_ptr<TOutputCapturer> StdOutCapturer;
    TInstant StartCurrentTestTime;
    void (*PrevAbortHandler)(int) = nullptr;
    void (*PrevSegvHandler)(int) = nullptr;
    std::optional<TCurrentTest> CurrentTest;
};

} // namespace NUnitTest
