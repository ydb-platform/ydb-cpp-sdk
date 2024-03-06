#include "log.h"
#include "element.h"
#include "stream.h"

#include <util/generic/string.h>
#include <util/stream/str.h>
#include <util/generic/ptr.h>
#include <utility>

#include <library/cpp/testing/unittest/registar.h>


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
    std::stringStream output;
    TLog log(std::make_unique<TStreamLogBackend>(&output));

    std::unique_ptr<TLogElement> src = std::make_unique<TLogElement>(&log);

    std::string message = "Hello, World!";
    (*src) << message;

    std::unique_ptr<TLogElement> dst = std::make_unique<TLogElement>(std::move(*src));

    src.Destroy();
    UNIT_ASSERT(output.Str() == "");

    dst.Destroy();
    UNIT_ASSERT(output.Str() == message);
}

void TLogElementTest::TestWith() {
    std::stringStream output;
    TLog log(std::make_unique<TStreamWithContextLogBackend>(&output));

    std::unique_ptr<TLogElement> src = std::make_unique<TLogElement>(&log);

    std::string message = "Hello, World!";
    (*src).With("Foo", "Bar").With("Foo", "Baz") << message;

    src.Destroy();
    UNIT_ASSERT(output.Str() == "Hello, World!; Foo=Bar; Foo=Baz; ");
}
