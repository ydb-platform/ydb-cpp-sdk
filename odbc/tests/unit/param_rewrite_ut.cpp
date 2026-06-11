#include "utils/bindings.h"
#include "utils/param_rewrite.h"

#include <gtest/gtest.h>

using NYdb::NOdbc::RewriteOdbcQuestionMarks;
using NYdb::NOdbc::TBoundParam;

namespace {

TBoundParam IntParam(SQLUSMALLINT n) {
    static SQLINTEGER value = 0;
    return {n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &value, 0, nullptr};
}

} // namespace

TEST(OdbcParamRewrite, RewritesQuestionMarks) {
    const std::vector<TBoundParam> params = {IntParam(1), IntParam(2)};
    const auto result = RewriteOdbcQuestionMarks("SELECT ? + ? AS result", params);
    ASSERT_TRUE(result.Success);
    EXPECT_EQ(result.Sql,
        "DECLARE $p1 AS Int32?;\n"
        "DECLARE $p2 AS Int32?;\n"
        "SELECT $p1 + $p2 AS result");
}

TEST(OdbcParamRewrite, SkipsLiteralAndYqlOptionalSyntax) {
    const std::vector<TBoundParam> params = {IntParam(1)};
    EXPECT_EQ(RewriteOdbcQuestionMarks("SELECT '?', ?", params).Sql,
        "DECLARE $p1 AS Int32?;\nSELECT '?', $p1");
    EXPECT_EQ(RewriteOdbcQuestionMarks("DECLARE $p1 AS Int32?;\nSELECT $p1", params).Sql,
        "DECLARE $p1 AS Int32?;\nSELECT $p1");
    EXPECT_EQ(RewriteOdbcQuestionMarks("SELECT $p1 + 10", params).Sql,
        "DECLARE $p1 AS Int32?;\nSELECT $p1 + 10");
}

TEST(OdbcParamRewrite, PrependsDeclareForNativeDollarParams) {
    const std::vector<TBoundParam> params = {IntParam(1), IntParam(2)};
    const auto result = RewriteOdbcQuestionMarks("SELECT $p1 + $p2 AS result", params);
    ASSERT_TRUE(result.Success);
    EXPECT_EQ(result.Sql,
        "DECLARE $p1 AS Int32?;\n"
        "DECLARE $p2 AS Int32?;\n"
        "SELECT $p1 + $p2 AS result");
}

TEST(OdbcParamRewrite, RejectsMismatchedBindCount) {
    const auto result = RewriteOdbcQuestionMarks("SELECT ? + ?", {IntParam(1)});
    ASSERT_FALSE(result.Success);
    EXPECT_EQ(result.SqlState, "07002");
}
