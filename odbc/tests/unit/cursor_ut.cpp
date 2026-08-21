#include "utils/cursor.h"

#include <gtest/gtest.h>

#include <limits>
#include <utility>

namespace NYdb::NOdbc {
namespace {

TEST(CursorWindow, PreservesScrollableRowsetPositioning) {
    TCursorWindow window(7);

    auto fetch = window.Fetch(SQL_FETCH_NEXT, 0, 3, 0);
    EXPECT_EQ(fetch.Rows, 3);
    EXPECT_FALSE(fetch.OverlappedStart);
    EXPECT_EQ(window.Resolve(0), 0);
    EXPECT_EQ(window.Resolve(2), 2);

    fetch = window.Fetch(SQL_FETCH_NEXT, 0, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 3);

    fetch = window.Fetch(SQL_FETCH_PRIOR, 0, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 1);

    fetch = window.Fetch(SQL_FETCH_LAST, 0, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 5);

    fetch = window.Fetch(SQL_FETCH_ABSOLUTE, -3, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 4);

    EXPECT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 0, 2, 0).Rows, 0);
    EXPECT_FALSE(window.Resolve(0));
    EXPECT_EQ(window.Fetch(SQL_FETCH_NEXT, 0, 2, 0).Rows, 2);
    EXPECT_EQ(window.Resolve(0), 0);

    EXPECT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 100, 2, 0).Rows, 0);
    EXPECT_EQ(window.Fetch(SQL_FETCH_PRIOR, 0, 2, 0).Rows, 2);
    EXPECT_EQ(window.Resolve(0), 5);

    ASSERT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 2, 2, 0).Rows, 2);
    fetch = window.Fetch(SQL_FETCH_PRIOR, 0, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_TRUE(fetch.OverlappedStart);
    EXPECT_EQ(window.Resolve(0), 0);

    fetch = window.Fetch(SQL_FETCH_ABSOLUTE, -1, 2, 0);
    EXPECT_EQ(fetch.Rows, 1);
    EXPECT_EQ(window.Resolve(0), 6);
    EXPECT_FALSE(window.Resolve(1));
}

TEST(CursorWindow, AppliesLimitBeforePositioning) {
    TCursorWindow window(8);
    auto fetch = window.Fetch(SQL_FETCH_LAST, 0, 3, 5);
    EXPECT_EQ(fetch.Rows, 3);
    EXPECT_EQ(window.Resolve(0), 2);
    EXPECT_EQ(window.Resolve(2), 4);

    EXPECT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 6, 3, 5).Rows, 0);
    EXPECT_EQ(window.Fetch(SQL_FETCH_PRIOR, 0, 3, 5).Rows, 3);
    EXPECT_EQ(window.Resolve(0), 2);
}

TEST(CursorWindow, HandlesEmptyAndExtremeOffsets) {
    TCursorWindow empty(0);
    EXPECT_EQ(empty.Fetch(SQL_FETCH_FIRST, 0, 1, 0).Rows, 0);
    EXPECT_EQ(empty.Fetch(SQL_FETCH_LAST, 0, 1, 0).Rows, 0);
    EXPECT_EQ(empty.Fetch(SQL_FETCH_ABSOLUTE,
                          std::numeric_limits<SQLLEN>::min(), 1, 0).Rows,
              0);

    TCursorWindow window(3);
    EXPECT_EQ(window.Fetch(SQL_FETCH_NEXT, 0, 0, 0).Rows, 0);
    EXPECT_FALSE(window.Resolve(0));
}

TEST(VirtualCursor, SharesIndexedPositioning) {
    const TColumnMeta column{"value", SQL_BIGINT, 19, SQL_NO_NULLS};
    TTable rows{{int64_t{1}}, {int64_t{2}}, {int64_t{3}}};
    auto cursor = CreateVirtualCursor(std::span(&column, 1), std::move(rows));
    ASSERT_EQ(cursor->Fetch(SQL_FETCH_LAST, 0, 1, 2).Rows, 1);
    SQLBIGINT value = 0;
    SQLLEN indicator = 0;
    ASSERT_EQ(cursor->GetData(0, 1, SQL_C_SBIGINT, &value, sizeof(value), &indicator),
              SQL_SUCCESS);
    EXPECT_EQ(value, 2);
}

} // namespace
} // namespace NYdb::NOdbc
