#include "log.h"
#include "element.h"
#include "stream.h"

#include <string>
#include <ydb-cpp-sdk/util/stream/str.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <utility>

#include <src/library/testing/unittest/registar.h>


class TLogElementTest: public TTestBase {
    UNIT_TEST_SUITE(TLogElementTest);
    UNIT_TEST(TestMoveCtor);
    UNIT_TEST(TestWith);
    UNIT_TEST_SUITE_END();

public:
    void TestMoveCtor();
    void TestWith();
};

UNIT_TEST_SUITE_REGISTRATION(TLogElementTest);

void TLogElementTest::TestMoveCtor() {
    std::stringstream output;
    TLog log(MakeHolder<TStreamLogBackend>(&output));

    THolder<TLogElement> src = MakeHolder<TLogElement>(&log);

    std::string message = "Hello, World!";
    (*src) << message;

    THolder<TLogElement> dst = MakeHolder<TLogElement>(std::move(*src));

    src.Destroy();
    UNIT_ASSERT(output.str() == "");

    dst.Destroy();
    UNIT_ASSERT(output.str() == message);
}

void TLogElementTest::TestWith() {
    std::stringstream output;
    TLog log(MakeHolder<TStreamWithContextLogBackend>(&output));

    THolder<TLogElement> src = MakeHolder<TLogElement>(&log);

    std::string message = "Hello, World!";
    (*src).With("Foo", "Bar").With("Foo", "Baz") << message;

    src.Destroy();
    UNIT_ASSERT(output.str() == "Hello, World!; Foo=Bar; Foo=Baz; ");
}
