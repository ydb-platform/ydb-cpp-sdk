#include "utils/sql_like.h"

#include <gtest/gtest.h>

using NYdb::NOdbc::SqlLikeMatch;

TEST(SqlLikeMatch, PercentMatchesSubstring) {
    EXPECT_TRUE(SqlLikeMatch("/local/foo_bar", "%foo%"));
    EXPECT_TRUE(SqlLikeMatch("/local/pfx_foo_sfx", "%foo%"));
    EXPECT_FALSE(SqlLikeMatch("/local/other", "%foo%"));
}

TEST(SqlLikeMatch, UnderscoreMatchesSingleChar) {
    EXPECT_TRUE(SqlLikeMatch("a_c", "a_c"));
    EXPECT_TRUE(SqlLikeMatch("abc", "a_c"));
    EXPECT_FALSE(SqlLikeMatch("abbc", "a_c"));
}

TEST(SqlLikeMatch, EmptyPatternMatchesOnlyEmptyText) {
    EXPECT_TRUE(SqlLikeMatch("", ""));
    EXPECT_FALSE(SqlLikeMatch("anything", ""));
}

TEST(SqlLikeMatch, PercentAtEnds) {
    EXPECT_TRUE(SqlLikeMatch("hello", "%hello%"));
    EXPECT_TRUE(SqlLikeMatch("hello", "hel%"));
    EXPECT_TRUE(SqlLikeMatch("hello", "%llo"));
}

TEST(SqlLikeMatch, BackslashEscapesWildcards) {
    EXPECT_TRUE(SqlLikeMatch("a_c", "a\\_c"));
    EXPECT_FALSE(SqlLikeMatch("abc", "a\\_c"));
    EXPECT_TRUE(SqlLikeMatch("rate%", "rate\\%"));
    EXPECT_FALSE(SqlLikeMatch("rate123", "rate\\%"));
    EXPECT_TRUE(SqlLikeMatch("a\\b", "a\\\\b"));
}

TEST(SqlLikeMatch, PercentBacktrackingHonorsEscapes) {
    EXPECT_TRUE(SqlLikeMatch("prefix_value_suffix", "%\\_value\\_%"));
    EXPECT_FALSE(SqlLikeMatch("prefixXvalueYsuffix", "%\\_value\\_%"));
}
