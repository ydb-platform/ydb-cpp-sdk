#include "utils/escape.h"

#include <gtest/gtest.h>

using NYdb::NOdbc::RewriteOdbcEscapes;

TEST(OdbcEscapeRewrite, FnUnwraps) {
    EXPECT_EQ(RewriteOdbcEscapes("SELECT {fn ABS(-12)} AS v"), "SELECT ABS(-12) AS v");
}

TEST(OdbcEscapeRewrite, FnCaseInsensitive) {
    EXPECT_EQ(RewriteOdbcEscapes("{FN LOWER('A')}"), "LOWER('A')");
}

TEST(OdbcEscapeRewrite, OjUnwraps) {
    EXPECT_EQ(RewriteOdbcEscapes("{oj LEFT OUTER JOIN t ON a=b}"), "LEFT OUTER JOIN t ON a=b");
}

TEST(OdbcEscapeRewrite, DateLiteral) {
    EXPECT_EQ(RewriteOdbcEscapes("SELECT {d '2024-01-01'}"), "SELECT CAST('2024-01-01' AS Date)");
}

TEST(OdbcEscapeRewrite, TimeLiteral) {
    EXPECT_EQ(RewriteOdbcEscapes("{t '14:30:00'}"), "CAST('14:30:00' AS Time)");
}

TEST(OdbcEscapeRewrite, TimestampLiteralNormalizesSpaceToT) {
    EXPECT_EQ(
        RewriteOdbcEscapes("SELECT {ts '2024-06-15 14:30:00'} AS v"),
        "SELECT CAST('2024-06-15T14:30:00Z' AS Datetime) AS v");
}

TEST(OdbcEscapeRewrite, TimestampLiteralKeepsExistingZ) {
    EXPECT_EQ(
        RewriteOdbcEscapes("SELECT {ts '2024-06-15T14:30:00Z'} AS v"),
        "SELECT CAST('2024-06-15T14:30:00Z' AS Datetime) AS v");
}

TEST(OdbcEscapeRewrite, Call) {
    EXPECT_EQ(RewriteOdbcEscapes("{call sp_demo(1, 2)}"), "CALL sp_demo(1, 2)");
}

TEST(OdbcEscapeRewrite, OutputCallBecomesPlainCall) {
    EXPECT_EQ(RewriteOdbcEscapes("{?= call sp(1)}"), "CALL sp(1)");
}

TEST(OdbcEscapeRewrite, EscapeClause) {
    EXPECT_EQ(RewriteOdbcEscapes("LIKE 'a%' {escape '\\'}"), "LIKE 'a%'  ESCAPE '\\'");
}

TEST(OdbcEscapeRewrite, ConvertOdbcToYqlCast) {
    EXPECT_EQ(
        RewriteOdbcEscapes("SELECT {fn CONVERT(42, SQL_SMALLINT)} AS v"),
        "SELECT CAST(42 AS Int16) AS v");
}

TEST(OdbcEscapeRewrite, ConvertNestedInFn) {
    EXPECT_EQ(RewriteOdbcEscapes("{fn CONVERT(x, SQL_INTEGER)}"), "CAST(x AS Int32)");
}

TEST(OdbcEscapeRewrite, NestedFnEscapes) {
    EXPECT_EQ(RewriteOdbcEscapes("{fn {fn ABS(1)}}"), "ABS(1)");
}

TEST(OdbcEscapeRewrite, UnknownBraceLeftUnchanged) {
    EXPECT_EQ(RewriteOdbcEscapes("{not_a_keyword 1}"), "{not_a_keyword 1}");
}

TEST(OdbcEscapeRewrite, EmptyInput) {
    EXPECT_EQ(RewriteOdbcEscapes(""), "");
}
