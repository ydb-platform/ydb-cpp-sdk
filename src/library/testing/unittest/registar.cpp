#include "registar.h"

#include <src/library/diff/diff.h>
#include <src/library/colorizer/colors.h>

#include <src/util/generic/bt_exception.h>
#include <src/util/random/fast.h>
#include <src/util/system/backtrace.h>
#include <src/util/system/guard.h>
#include <src/util/system/tls.h>
#include <src/util/system/error.h>
#include <src/util/string/cast.h>

#include <mutex>

using namespace std::string_literals;

bool NUnitTest::ShouldColorizeDiff = true;

std::string NUnitTest::RandomString(size_t len, ui32 seed) {
    TReallyFastRng32 rand(seed);
    std::string ret;

    ret.reserve(len);

    for (size_t i = 0; i < len; ++i) {
        ret.push_back(char(rand.Uniform(1, 128)));
    }

    return ret;
}

Y_POD_STATIC_THREAD(bool)
UnittestThread;
Y_POD_STATIC_THREAD(NUnitTest::TTestBase*)
currentTest;
::NUnitTest::TRaiseErrorHandler RaiseErrorHandler;

void ::NUnitTest::NPrivate::RaiseError(const char* what, const std::string& msg, bool fatalFailure) {
    Y_ABORT_UNLESS(UnittestThread, "%s in non-unittest thread with message:\n%s", what, msg.data());
    Y_ABORT_UNLESS(GetCurrentTest());

    if (RaiseErrorHandler) {
        RaiseErrorHandler(what, msg, fatalFailure);
        return;
    }

    // Default handler
    TBackTrace bt;
    bt.Capture();
    GetCurrentTest()->AddError(msg.data(), bt.PrintToString());
    if (!fatalFailure) {
        return;
    }
    throw TAssertException();
}

void ::NUnitTest::SetRaiseErrorHandler(::NUnitTest::TRaiseErrorHandler handler) {
    Y_ABORT_UNLESS(UnittestThread);
    RaiseErrorHandler = std::move(handler);
}

void ::NUnitTest::NPrivate::SetUnittestThread(bool unittestThread) {
    Y_ABORT_UNLESS(UnittestThread != unittestThread, "state check");
    UnittestThread = unittestThread;
}

void ::NUnitTest::NPrivate::SetCurrentTest(TTestBase* test) {
    Y_ABORT_UNLESS(!test || !currentTest, "state check");
    currentTest = test;
}

NUnitTest::TTestBase* ::NUnitTest::NPrivate::GetCurrentTest() {
    return currentTest;
}

struct TDiffColorizer {
    NColorizer::TColors Colors;
    bool Reverse = false;

    explicit TDiffColorizer(bool reverse = false)
        : Reverse(reverse)
    {
    }

    std::string Special(std::string_view str) const {
        return ToString(Colors.YellowColor()) += str;
    }

    std::string Common(std::span<const char> str) const {
        return ToString(Colors.OldColor()) + std::string(str.begin(), str.end());
    }

    std::string Left(std::span<const char> str) const {
        return ToString(GetLeftColor()) + std::string(str.begin(), str.end());
    }

    std::string Right(std::span<const char> str) const {
        return ToString(GetRightColor()) + std::string(str.begin(), str.end());
    }

    std::string_view GetLeftColor() const {
        return Reverse ? Colors.RedColor() : Colors.GreenColor();
    }

    std::string_view GetRightColor() const {
        return Reverse ? Colors.GreenColor() : Colors.RedColor();
    }
};

struct TTraceDiffFormatter {
    bool Reverse = false;

    explicit TTraceDiffFormatter(bool reverse = false)
        : Reverse(reverse)
    {
    }

    std::string Special(std::string_view str) const {
        return ToString(str);
    }

    std::string Common(std::span<const char> str) const {
        return std::string(str.begin(), str.end());
    }

    std::string Left(std::span<const char> str) const {
        return NUnitTest::GetFormatTag("good") +
               std::string(str.begin(), str.end()) +
               NUnitTest::GetResetTag();
    }

    std::string Right(std::span<const char> str) const {
        return NUnitTest::GetFormatTag("bad") +
               std::string(str.begin(), str.end()) +
               NUnitTest::GetResetTag();
    }
};

std::string NUnitTest::GetFormatTag(const char* name) {
    return std::format("[[{}]]", name);
}

std::string NUnitTest::GetResetTag() {
    return "[[rst]]"s;
}

std::string NUnitTest::ColoredDiff(std::string_view s1, std::string_view s2, const std::string& delims, bool reverse) {
    TStringStream res;
    std::vector<NDiff::TChunk<char>> chunks;
    NDiff::InlineDiff(chunks, s1, s2, delims);
    if (NUnitTest::ShouldColorizeDiff) {
        NDiff::PrintChunks(res, TDiffColorizer(reverse), chunks);
    } else {
        res << NUnitTest::GetResetTag();
        NDiff::PrintChunks(res, TTraceDiffFormatter(reverse), chunks);
    }
    return res.Str();
}

static std::string MakeTestName(const NUnitTest::ITestSuiteProcessor::TTest& test) {
    return TStringBuilder() << test.unit->name << "::" << test.name;
}

static size_t CountTests(const std::map<std::string, size_t>& testErrors, bool succeeded) {
    size_t cnt = 0;
    for (const auto& t : testErrors) {
        if (succeeded && t.second == 0) {
            ++cnt;
        } else if (!succeeded && t.second > 0) {
            ++cnt;
        }
    }
    return cnt;
}

NUnitTest::ITestSuiteProcessor::ITestSuiteProcessor() = default;

NUnitTest::ITestSuiteProcessor::~ITestSuiteProcessor() = default;

void NUnitTest::ITestSuiteProcessor::Start() {
    OnStart();
}

void NUnitTest::ITestSuiteProcessor::End() {
    OnEnd();
}

void NUnitTest::ITestSuiteProcessor::UnitStart(const TUnit& unit) {
    CurTestErrors_.clear();

    OnUnitStart(&unit);
}

void NUnitTest::ITestSuiteProcessor::UnitStop(const TUnit& unit) {
    OnUnitStop(&unit);
}

void NUnitTest::ITestSuiteProcessor::Error(const TError& descr) {
    AddTestError(*descr.test);

    OnError(&descr);
}

void NUnitTest::ITestSuiteProcessor::BeforeTest(const TTest& test) {
    OnBeforeTest(&test);
}

void NUnitTest::ITestSuiteProcessor::Finish(const TFinish& descr) {
    AddTestFinish(*descr.test);

    OnFinish(&descr);
}

unsigned NUnitTest::ITestSuiteProcessor::GoodTests() const noexcept {
    return CountTests(TestErrors_, true);
}

unsigned NUnitTest::ITestSuiteProcessor::FailTests() const noexcept {
    return CountTests(TestErrors_, false);
}

unsigned NUnitTest::ITestSuiteProcessor::GoodTestsInCurrentUnit() const noexcept {
    return CountTests(CurTestErrors_, true);
}

unsigned NUnitTest::ITestSuiteProcessor::FailTestsInCurrentUnit() const noexcept {
    return CountTests(CurTestErrors_, false);
}

bool NUnitTest::ITestSuiteProcessor::CheckAccess(std::string /*name*/, size_t /*num*/) {
    return true;
}

bool NUnitTest::ITestSuiteProcessor::CheckAccessTest(std::string /*suite*/, const char* /*name*/) {
    return true;
}

void NUnitTest::ITestSuiteProcessor::Run(std::function<void()> f, const std::string& /*suite*/, const char* /*name*/, const bool /*forceFork*/) {
    f();
}

void NUnitTest::ITestSuiteProcessor::SetForkTestsParams(bool forkTests, bool isForked) {
    ForkTests_ = forkTests;
    IsForked_ = isForked;
}

bool NUnitTest::ITestSuiteProcessor::GetIsForked() const {
    return IsForked_;
}

bool NUnitTest::ITestSuiteProcessor::GetForkTests() const {
    return ForkTests_;
}

void NUnitTest::ITestSuiteProcessor::OnStart() {
}

void NUnitTest::ITestSuiteProcessor::OnEnd() {
}

void NUnitTest::ITestSuiteProcessor::OnUnitStart(const TUnit* /*unit*/) {
}

void NUnitTest::ITestSuiteProcessor::OnUnitStop(const TUnit* /*unit*/) {
}

void NUnitTest::ITestSuiteProcessor::OnError(const TError* /*error*/) {
}

void NUnitTest::ITestSuiteProcessor::OnFinish(const TFinish* /*finish*/) {
}

void NUnitTest::ITestSuiteProcessor::OnBeforeTest(const TTest* /*test*/) {
}

void NUnitTest::ITestSuiteProcessor::AddTestError(const TTest& test) {
    const std::string name = MakeTestName(test);
    ++TestErrors_[name];
    ++CurTestErrors_[name];
}

void NUnitTest::ITestSuiteProcessor::AddTestFinish(const TTest& test) {
    const std::string name = MakeTestName(test);
    TestErrors_[name];    // zero errors if not touched
    CurTestErrors_[name]; // zero errors if not touched
}

NUnitTest::ITestBaseFactory::ITestBaseFactory() {
    Register();
}

NUnitTest::ITestBaseFactory::~ITestBaseFactory() = default;

void NUnitTest::ITestBaseFactory::Register() noexcept {
    TTestFactory::Instance().Register(this);
}

NUnitTest::TTestBase::TTestBase() noexcept
    : Parent_(nullptr)
    , TestErrors_()
    , CurrentSubtest_()
{
}

NUnitTest::TTestBase::~TTestBase() = default;

std::string NUnitTest::TTestBase::TypeId() const {
    return TypeName(*this);
}

void NUnitTest::TTestBase::SetUp() {
}

void NUnitTest::TTestBase::TearDown() {
}

void NUnitTest::TTestBase::AddError(const char* msg, const std::string& backtrace, TTestContext* context) {
    ++TestErrors_;
    const NUnitTest::ITestSuiteProcessor::TUnit unit = {Name()};
    const NUnitTest::ITestSuiteProcessor::TTest test = {&unit, CurrentSubtest_};
    const NUnitTest::ITestSuiteProcessor::TError err = {&test, msg, backtrace, context};

    Processor()->Error(err);
}

void NUnitTest::TTestBase::AddError(const char* msg, TTestContext* context) {
    AddError(msg, ""s, context);
}

void NUnitTest::TTestBase::RunAfterTest(std::function<void()> f) {
    std::lock_guard guard(AfterTestFunctionsLock_);
    AfterTestFunctions_.emplace_back(std::move(f));
}

bool NUnitTest::TTestBase::CheckAccessTest(const char* test) {
    return Processor()->CheckAccessTest(Name(), test);
}

void NUnitTest::TTestBase::BeforeTest(const char* func) {
    const NUnitTest::ITestSuiteProcessor::TUnit unit = {Name()};
    const NUnitTest::ITestSuiteProcessor::TTest test = {&unit, func};
    rusage.Fill();
    Processor()->BeforeTest(test);
}

void NUnitTest::TTestBase::Finish(const char* func, TTestContext* context) {
    TRusage finishRusage = TRusage::Get();
    context->Metrics["ru_rss"] = finishRusage.MaxRss - rusage.MaxRss;
    context->Metrics["ru_major_pagefaults"] = finishRusage.MajorPageFaults - rusage.MajorPageFaults;
    context->Metrics["ru_utime"] = (finishRusage.Utime - rusage.Utime).MicroSeconds();
    context->Metrics["ru_stime"] = (finishRusage.Stime - rusage.Stime).MicroSeconds();

    const NUnitTest::ITestSuiteProcessor::TUnit unit = {Name()};
    const NUnitTest::ITestSuiteProcessor::TTest test = {&unit, func};
    const NUnitTest::ITestSuiteProcessor::TFinish finish = {&test, context, TestErrors_ == 0};

    Processor()->Finish(finish);
}

void NUnitTest::TTestBase::AtStart() {
    const NUnitTest::ITestSuiteProcessor::TUnit unit = {Name()};

    Processor()->UnitStart(unit);
}

void NUnitTest::TTestBase::AtEnd() {
    const NUnitTest::ITestSuiteProcessor::TUnit unit = {Name()};

    Processor()->UnitStop(unit);
}

void NUnitTest::TTestBase::Run(std::function<void()> f, const std::string& suite, const char* name, const bool forceFork) {
    TestErrors_ = 0;
    CurrentSubtest_ = name;
    Processor()->Run(f, suite, name, forceFork);
}

void NUnitTest::TTestBase::BeforeTest() {
    SetUp();
}

void NUnitTest::TTestBase::AfterTest() {
    TearDown();

    std::vector<std::function<void()>> afterTestFunctions;
    {
        std::lock_guard guard(AfterTestFunctionsLock_);
        afterTestFunctions.swap(AfterTestFunctions_);
    }

    for (auto i = afterTestFunctions.rbegin(); i != afterTestFunctions.rend(); ++i) {
        std::function<void()>& f = *i;
        if (f) {
            f();
        }
    }
}

bool NUnitTest::TTestBase::GetIsForked() const {
    return Processor()->GetIsForked();
}

bool NUnitTest::TTestBase::GetForkTests() const {
    return Processor()->GetForkTests();
}

NUnitTest::ITestSuiteProcessor* NUnitTest::TTestBase::Processor() const noexcept {
    return Parent_->Processor();
}

NUnitTest::TTestBase::TCleanUp::TCleanUp(TTestBase* base)
    : Base_(base)
{
    ::NUnitTest::NPrivate::SetCurrentTest(base);
    ::NUnitTest::NPrivate::SetUnittestThread(true);
    Base_->BeforeTest();
}

NUnitTest::TTestBase::TCleanUp::~TCleanUp() {
    try {
        Base_->AfterTest();
    } catch (...) {
        Base_->AddError(CurrentExceptionMessage().data());
    }
    ::NUnitTest::NPrivate::SetUnittestThread(false);
    ::NUnitTest::NPrivate::SetCurrentTest(nullptr);
}

namespace {
    /*
     * by default do nothing
     */
    class TCommonProcessor: public NUnitTest::ITestSuiteProcessor {
    public:
        TCommonProcessor() = default;

        ~TCommonProcessor() override = default;
    };

    struct TCmp {
        template <class T>
        inline bool operator()(const T& l, const T& r) const noexcept {
            return stricmp(Fix(l.Name().data()), Fix(r.Name().data())) < 0;
        }

        static inline const char* Fix(const char* n) noexcept {
            if (*n == 'T') {
                return n + 1;
            }

            return n;
        }
    };
}

NUnitTest::TTestFactory::TTestFactory(ITestSuiteProcessor* processor)
    : Processor_(processor)
{
}

NUnitTest::TTestFactory::~TTestFactory() = default;

NUnitTest::TTestFactory& NUnitTest::TTestFactory::Instance() {
    static TCommonProcessor p;
    static TTestFactory f(&p);

    return f;
}

unsigned NUnitTest::TTestFactory::Execute() {
    Items_.QuickSort(TCmp());
    Processor_->Start();

    TSet<std::string> types;
    size_t cnt = 0;

    for (TIntrusiveList<ITestBaseFactory>::TIterator factory = Items_.Begin(); factory != Items_.End(); ++factory) {
        if (!Processor_->CheckAccess(factory->Name(), cnt++)) {
            continue;
        }

        std::unique_ptr<TTestBase> test(factory->ConstructTest());

#ifdef _unix_ // on Windows RTTI causes memory leaks
        std::string type = test->TypeId();
        if (types.insert(type).second == false) {
            warnx("Duplicate suite found: %s (%s). Probably you have copy-pasted suite without changing it name", factory->Name().c_str(), type.c_str());
            return 1;
        }
#endif // _unix_

        test->Parent_ = this;

#ifdef UT_SKIP_EXCEPTIONS
        try {
#endif
            test->Execute();
#ifdef UT_SKIP_EXCEPTIONS
        } catch (...) {
        }
#endif
    }

    Processor_->End();

    return bool(Processor_->FailTests());
}

void NUnitTest::TTestFactory::SetProcessor(ITestSuiteProcessor* processor) {
    Processor_ = processor;
}

void NUnitTest::TTestFactory::Register(ITestBaseFactory* b) noexcept {
    Items_.PushBack(b);
}

NUnitTest::ITestSuiteProcessor* NUnitTest::TTestFactory::Processor() const noexcept {
    return Processor_;
}
