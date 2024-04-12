#include "events.h"

#include <src/library/testing/unittest/registar.h>

#include <util/string/builder.h>

#include <string>
#include <string_view>

void TGMockTestEventListener::OnTestPartResult(const testing::TestPartResult& result) {
    using namespace std::string_view_literals;

    if (result.failed()) {
        const std::string message = result.message();
        const std::string summary = result.summary();
        TStringBuilder msg;
        if (result.file_name())
            msg << result.file_name() << ":"sv;
        if (result.line_number() != -1)
            msg << result.line_number() << ":"sv;
        if (!summary.empty()) {
            if (!msg.empty()) {
                msg << "\n"sv;
            }
            msg << summary;
        }
        if (!message.empty() && summary != message) {
            if (!msg.empty()) {
                msg << "\n"sv;
            }
            msg << message;
        }
        NUnitTest::NPrivate::RaiseError(result.summary(), msg, result.fatally_failed());
    }
}
