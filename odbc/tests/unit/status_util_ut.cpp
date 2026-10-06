#include "utils/status_util.h"

#include <gtest/gtest.h>

namespace NYdb::NOdbc {
namespace {

TEST(SessionInvalidation, MatchesSdkBrokenSessionStatuses) {
    for (const auto status : {EStatus::BAD_SESSION, EStatus::SESSION_BUSY,
            EStatus::CLIENT_DEADLINE_EXCEEDED, EStatus::CLIENT_CANCELLED,
            EStatus::TRANSPORT_UNAVAILABLE}) {
        EXPECT_TRUE(IsSessionInvalidated(TStatus(status, NIssue::TIssues())))
            << static_cast<int>(status);
    }
}

TEST(SessionInvalidation, DoesNotReleaseUnconfirmedLeases) {
    for (const auto status : {EStatus::SUCCESS, EStatus::TIMEOUT,
            EStatus::SESSION_EXPIRED, EStatus::CANCELLED, EStatus::UNDETERMINED,
            EStatus::CLIENT_RESOURCE_EXHAUSTED, EStatus::CLIENT_OUT_OF_RANGE}) {
        EXPECT_FALSE(IsSessionInvalidated(TStatus(status, NIssue::TIssues())))
            << static_cast<int>(status);
    }
}

} // namespace
} // namespace NYdb::NOdbc
