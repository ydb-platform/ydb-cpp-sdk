#include <util/generic/set.h>
#include <string>
#include <string_view>
#include <utility>

#include <library/cpp/http/io/headers.h>
#include <library/cpp/testing/unittest/registar.h>

namespace {
    class THeadersExistence {
    public:
        THeadersExistence() = default;

        THeadersExistence(const THttpHeaders& headers) {
            for (THttpHeaders::TConstIterator it = headers.Begin();
                 it != headers.End();
                 ++it) {
                Add(it->Name(), it->Value());
            }
        }

    public:
        void Add(std::string_view name, std::string_view value) {
            Impl.emplace(std::string(name), std::string(value));
        }

        bool operator==(const THeadersExistence& rhs) const {
            return Impl == rhs.Impl;
        }

    private:
        typedef TMultiSet<std::pair<std::string, std::string>> TImpl;
        TImpl Impl;
    };
}

bool operator==(const THeadersExistence& lhs, const THttpHeaders& rhs) {
    return lhs == THeadersExistence(rhs);
}

bool operator==(const THttpHeaders& lhs, const THeadersExistence& rhs) {
    return THeadersExistence(lhs) == rhs;
}

class THttpHeadersTest: public TTestBase {
    UNIT_TEST_SUITE(THttpHeadersTest);
    UNIT_TEST(TestAddOperation1Arg);
    UNIT_TEST(TestAddOperation2Args);
    UNIT_TEST(TestAddOrReplaceOperation1Arg);
    UNIT_TEST(TestAddOrReplaceOperation2Args);
    UNIT_TEST(TestAddHeaderTemplateness);
    UNIT_TEST(TestFindHeader);
    UNIT_TEST_SUITE_END();

private:
    typedef void (*TAddHeaderFunction)(THttpHeaders&, std::string_view name, std::string_view value);
    typedef void (*TAddOrReplaceHeaderFunction)(THttpHeaders&, std::string_view name, std::string_view value);

public:
    void TestAddOperation1Arg();
    void TestAddOperation2Args();
    void TestAddOrReplaceOperation1Arg();
    void TestAddOrReplaceOperation2Args();
    void TestAddHeaderTemplateness();
    void TestFindHeader();

private:
    static void AddHeaderImpl1Arg(THttpHeaders& headers, std::string_view name, std::string_view value) {
        headers.AddHeader(THttpInputHeader(std::string(name), std::string(value)));
    }

    static void AddHeaderImpl2Args(THttpHeaders& headers, std::string_view name, std::string_view value) {
        headers.AddHeader(std::string(name), std::string(value));
    }

    static void AddOrReplaceHeaderImpl1Arg(THttpHeaders& headers, std::string_view name, std::string_view value) {
        headers.AddOrReplaceHeader(THttpInputHeader(std::string(name), std::string(value)));
    }

    static void AddOrReplaceHeaderImpl2Args(THttpHeaders& headers, std::string_view name, std::string_view value) {
        headers.AddOrReplaceHeader(std::string(name), std::string(value));
    }

    void DoTestAddOperation(TAddHeaderFunction);
    void DoTestAddOrReplaceOperation(TAddHeaderFunction, TAddOrReplaceHeaderFunction);
};

UNIT_TEST_SUITE_REGISTRATION(THttpHeadersTest);

void THttpHeadersTest::TestAddOperation1Arg() {
    DoTestAddOperation(AddHeaderImpl1Arg);
}
void THttpHeadersTest::TestAddOperation2Args() {
    DoTestAddOperation(AddHeaderImpl2Args);
}

void THttpHeadersTest::TestAddOrReplaceOperation1Arg() {
    DoTestAddOrReplaceOperation(AddHeaderImpl1Arg, AddOrReplaceHeaderImpl1Arg);
}
void THttpHeadersTest::TestAddOrReplaceOperation2Args() {
    DoTestAddOrReplaceOperation(AddHeaderImpl2Args, AddOrReplaceHeaderImpl2Args);
}

void THttpHeadersTest::DoTestAddOperation(TAddHeaderFunction addHeader) {
    THttpHeaders h1;

    addHeader(h1, "h1", "v1");
    addHeader(h1, "h2", "v1");

    addHeader(h1, "h3", "v1");
    addHeader(h1, "h3", "v2");
    addHeader(h1, "h3", "v2");

    THeadersExistence h2;

    h2.Add("h1", "v1");
    h2.Add("h2", "v1");

    h2.Add("h3", "v1");
    h2.Add("h3", "v2");
    h2.Add("h3", "v2");

    UNIT_ASSERT(h2 == h1);
}

// Sorry, but AddOrReplaceHeader replaces only first occurence
void THttpHeadersTest::DoTestAddOrReplaceOperation(TAddHeaderFunction addHeader, TAddOrReplaceHeaderFunction addOrReplaceHeader) {
    THttpHeaders h1;

    addHeader(h1, "h1", "v1");

    addOrReplaceHeader(h1, "h2", "v1");
    addOrReplaceHeader(h1, "h2", "v2");
    addOrReplaceHeader(h1, "h2", "v3");
    addHeader(h1, "h2", "v4");

    addHeader(h1, "h3", "v1");
    addHeader(h1, "h3", "v2");
    addOrReplaceHeader(h1, "h3", "v3");

    THeadersExistence h2;

    h2.Add("h1", "v1");

    h2.Add("h2", "v3");
    h2.Add("h2", "v4");

    h2.Add("h3", "v2");
    h2.Add("h3", "v3");

    UNIT_ASSERT(h2 == h1);
}

void THttpHeadersTest::TestAddHeaderTemplateness() {
    THttpHeaders h1;
    h1.AddHeader("h1", "v1");
    h1.AddHeader("h2", std::string("v2"));
    h1.AddHeader("h3", std::string_view("v3"));
    h1.AddHeader("h4", std::string_view("v4"));

    THeadersExistence h2;
    h2.Add("h1", "v1");
    h2.Add("h2", "v2");
    h2.Add("h3", "v3");
    h2.Add("h4", "v4");

    UNIT_ASSERT(h1 == h2);
}

void THttpHeadersTest::TestFindHeader() {
    THttpHeaders sut;
    sut.AddHeader("NaMe", "Value");

    UNIT_ASSERT(sut.FindHeader("name"));
    UNIT_ASSERT(sut.FindHeader("name")->Value() == "Value");
}
