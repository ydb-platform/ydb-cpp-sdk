#include "gtest.h"
#include "simple.h"

#include <ydb-cpp-sdk/util/system/type_name.h>

using namespace NUnitTest;
using namespace NUnitTest::NPrivate;

IGTestFactory::~IGTestFactory() {
}

namespace {
    struct TCurrentTest: public TSimpleTestExecutor {
        inline TCurrentTest(std::string_view name)
            : MyName(name)
        {
        }

        std::string TypeId() const override {
            return (TypeName(*this) += '-') += MyName;
        }

        std::string Name() const noexcept override {
            return std::string(MyName);
        }

        const std::string_view MyName;
    };

    struct TGTestFactory: public IGTestFactory {
        inline TGTestFactory(std::string_view name)
            : Test(name)
        {
        }

        ~TGTestFactory() override {
        }

        std::string Name() const noexcept override {
            return Test.Name();
        }

        TTestBase* ConstructTest() override {
            return new TCurrentTest(Test);
        }

        void AddTest(const char* name, void (*body)(TTestContext&), bool forceFork) override {
            Test.Tests.push_back(TBaseTestCase(name, body, forceFork));
        }

        TCurrentTest Test;
    };
}

IGTestFactory* NUnitTest::NPrivate::ByName(const char* name) {
    static std::map<std::string_view, TAutoPtr<TGTestFactory>> tests;

    auto& ret = tests[name];

    if (!ret) {
        ret = new TGTestFactory(name);
    }

    return ret.Get();
}
