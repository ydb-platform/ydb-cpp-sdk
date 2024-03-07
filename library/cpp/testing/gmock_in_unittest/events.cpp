#include "events.h"

#include <library/cpp/testing/unittest/registar.h>

#include <string_view>
#include <string>
#include <util/string/builder.h>

void TGMockTestEventListener::OnTestPartResult(const testing::TestPartResult& result) {
    if (result.failed()) {
        const std::string message = result.message();
        const std::string summary = result.summary();
        TYdbStringBuilder msg;
        if (result.file_name())
            msg << result.file_name() << std::string_view(":");
        if (result.line_number() != -1)
            msg << result.line_number() << std::string_view(":");
        if (summary) {
            if (msg) {
                msg << std::string_view("\n");
            }
            msg << summary;
        }
        if (message && summary != message) {
            if (msg) {
                msg << std::string_view("\n");
            }
            msg << message;
        }
        NUnitTest::NPrivate::RaiseError(result.summary(), msg, result.fatally_failed());
    }
}
