#include "rty_formater.h"

#include <library/cpp/testing/unittest/registar.h>

namespace {
    const std::string_view SampleISO8601("2017-07-25T19:26:09.894000+03:00");
    const std::string_view SampleRtyLog("2017-07-25 19:26:09.894 +0300");
}

Y_UNIT_TEST_SUITE(NLoggingImplTest) {
    Y_UNIT_TEST(TestTLocalTimeSToStream) {
        NLoggingImpl::TLocalTimeS lt(TInstant::ParseIso8601Deprecated(SampleISO8601));
        std::stringStream ss;
        ss << lt;
        UNIT_ASSERT_EQUAL(ss.Str(), SampleRtyLog);
    }
    Y_UNIT_TEST(TestTLocalTimeSToString) {
        NLoggingImpl::TLocalTimeS lt(TInstant::ParseIso8601Deprecated(SampleISO8601));
        UNIT_ASSERT_EQUAL(std::string(lt), SampleRtyLog);
    }
    Y_UNIT_TEST(TestTLocalTimeSAddLeft) {
        NLoggingImpl::TLocalTimeS lt(TInstant::ParseIso8601Deprecated(SampleISO8601));
        std::string_view suffix("suffix");
        UNIT_ASSERT_EQUAL(lt + suffix, std::string(SampleRtyLog) + suffix);
    }
    Y_UNIT_TEST(TestTLocalTimeSAddRight) {
        NLoggingImpl::TLocalTimeS lt(TInstant::ParseIso8601Deprecated(SampleISO8601));
        std::string prefix("prefix");
        UNIT_ASSERT_EQUAL(prefix + lt, prefix + SampleRtyLog);
    }
}
