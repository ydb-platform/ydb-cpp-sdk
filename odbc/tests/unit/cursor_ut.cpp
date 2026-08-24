#include "utils/cursor_window.h"

#include <gtest/gtest.h>

#include <limits>

namespace NYdb::NOdbc {
namespace {

TEST(CursorWindow, PreservesScrollableRowsetPositioning) {
    TCursorWindow window(7);
    EXPECT_EQ(window.RowNumber(), 0);

    auto fetch = window.Fetch(SQL_FETCH_NEXT, 0, 3, 0);
    EXPECT_EQ(fetch.Rows, 3);
    EXPECT_FALSE(fetch.OverlappedStart);
    EXPECT_EQ(window.Resolve(0), 0);
    EXPECT_EQ(window.Resolve(2), 2);
    EXPECT_EQ(window.RowNumber(), 1);

    fetch = window.Fetch(SQL_FETCH_NEXT, 0, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 3);
    EXPECT_EQ(window.RowNumber(), 4);

    fetch = window.Fetch(SQL_FETCH_PRIOR, 0, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 1);
    EXPECT_EQ(window.RowNumber(), 2);

    fetch = window.Fetch(SQL_FETCH_LAST, 0, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 5);
    EXPECT_EQ(window.RowNumber(), 6);

    fetch = window.Fetch(SQL_FETCH_ABSOLUTE, -3, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_EQ(window.Resolve(0), 4);

    EXPECT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 0, 2, 0).Rows, 0);
    EXPECT_FALSE(window.Resolve(0));
    EXPECT_EQ(window.RowNumber(), 0);
    EXPECT_EQ(window.Fetch(SQL_FETCH_NEXT, 0, 2, 0).Rows, 2);
    EXPECT_EQ(window.Resolve(0), 0);

    EXPECT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 100, 2, 0).Rows, 0);
    EXPECT_EQ(window.RowNumber(), 0);
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

TEST(CursorWindow, ResolvesRelativePositionsAtBoundaries) {
    TCursorWindow window(7);

    EXPECT_EQ(window.Fetch(SQL_FETCH_RELATIVE, -1, 2, 0).Rows, 0);
    EXPECT_EQ(window.Fetch(SQL_FETCH_RELATIVE, 2, 2, 0).Rows, 2);
    EXPECT_EQ(window.Resolve(0), 1);

    auto fetch = window.Fetch(SQL_FETCH_RELATIVE, -2, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_TRUE(fetch.OverlappedStart);
    EXPECT_EQ(window.Resolve(0), 0);

    EXPECT_EQ(window.Fetch(SQL_FETCH_RELATIVE, 4, 2, 0).Rows, 2);
    EXPECT_EQ(window.Resolve(0), 4);

    EXPECT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 100, 2, 0).Rows, 0);
    EXPECT_EQ(window.Fetch(SQL_FETCH_RELATIVE, -2, 2, 0).Rows, 2);
    EXPECT_EQ(window.Resolve(0), 5);
}

TEST(CursorWindow, DetectsRelativeRowsetsOverlappingTheBeginning) {
    TCursorWindow window(10);

    ASSERT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 6, 2, 0).Rows, 2);
    auto fetch = window.Fetch(SQL_FETCH_RELATIVE, -6, 2, 0);
    EXPECT_EQ(fetch.Rows, 2);
    EXPECT_TRUE(fetch.OverlappedStart);
    EXPECT_EQ(window.Resolve(0), 0);
    EXPECT_EQ(window.Resolve(1), 1);

    ASSERT_EQ(window.Fetch(SQL_FETCH_ABSOLUTE, 6, 2, 0).Rows, 2);
    fetch = window.Fetch(SQL_FETCH_RELATIVE, -7, 2, 0);
    EXPECT_EQ(fetch.Rows, 0);
    EXPECT_FALSE(fetch.OverlappedStart);
    EXPECT_FALSE(window.Resolve(0));
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

} // namespace
} // namespace NYdb::NOdbc
