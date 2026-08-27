#include "utils/bindings.h"
#include "utils/param_rewrite.h"

#include <gtest/gtest.h>

using NYdb::NOdbc::RewriteOdbcSql;
using NYdb::NOdbc::CountOdbcParams;
using NYdb::NOdbc::StartsWithSqlStatement;
using NYdb::NOdbc::TBoundParam;

namespace {

TBoundParam IntParam(SQLUSMALLINT n) {
    static SQLINTEGER value = 0;
    return {n, SQL_C_LONG, SQL_INTEGER, 0, 0, &value, 0, nullptr};
}

TBoundParam NullIntParam(SQLUSMALLINT n) {
    static SQLINTEGER value = 0;
    static SQLLEN indicator = SQL_NULL_DATA;
    return {n, SQL_C_LONG, SQL_INTEGER, 0, 0, &value, 0, &indicator};
}

NYdb::NOdbc::TParamRewriteResult RewriteParams(
    std::string_view sql,
    const std::vector<TBoundParam>& params) {
    return RewriteOdbcSql(sql, params, false);
}

} // namespace

TEST(OdbcParamRewrite, RewritesQuestionMarks) {
    const std::vector<TBoundParam> params = {IntParam(1), IntParam(2)};
    const auto result = RewriteParams("SELECT ? + ? AS result", params);
    ASSERT_TRUE(result.Success);
    EXPECT_EQ(result.Sql,
        "DECLARE $p1 AS Int32;\n"
        "DECLARE $p2 AS Int32;\n"
        "SELECT $p1 + $p2 AS result");
}

TEST(OdbcParamRewrite, RewritesEscapesAndParametersInOnePass) {
    const auto result = RewriteOdbcSql(
        "SELECT {fn CONVERT(?, SQL_INTEGER)}", {IntParam(1)}, true);
    ASSERT_TRUE(result.Success);
    EXPECT_EQ(result.Sql, "DECLARE $p1 AS Int32;\nSELECT CAST($p1 AS Int32)");
}

TEST(OdbcParamRewrite, UsesBoundCTypeForYdbDeclaration) {
    SQLUBIGINT value = 42;
    const std::vector<TBoundParam> params = {{
        1, SQL_C_UBIGINT, SQL_BIGINT, 0, 0,
        &value, sizeof(value), nullptr
    }};
    EXPECT_EQ(RewriteParams("SELECT ?", params).Sql,
              "DECLARE $p1 AS Uint64;\nSELECT $p1");
}

TEST(OdbcParamRewrite, PreservesTemporalAndDecimalTypes) {
    SQL_TIMESTAMP_STRUCT timestamp{};
    SQLDOUBLE decimal = 0;
    const std::vector<TBoundParam> params = {
        {1, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0,
         &timestamp, sizeof(timestamp), nullptr},
        {2, SQL_C_DOUBLE, SQL_DECIMAL, 18, 5,
         &decimal, sizeof(decimal), nullptr},
    };
    EXPECT_EQ(RewriteParams("SELECT ?, ?", params).Sql,
              "DECLARE $p1 AS Timestamp;\n"
              "DECLARE $p2 AS Decimal(18, 5);\n"
              "SELECT $p1, $p2");
}

TEST(OdbcParamRewrite, SkipsLiteralAndYqlOptionalSyntax) {
    const std::vector<TBoundParam> params = {IntParam(1)};
    EXPECT_EQ(RewriteParams("SELECT '?', ?", params).Sql,
        "DECLARE $p1 AS Int32;\nSELECT '?', $p1");
    EXPECT_EQ(RewriteParams("DECLARE $p1 AS Int32?;\nSELECT $p1", params).Sql,
        "DECLARE $p1 AS Int32?;\nSELECT $p1");
    EXPECT_EQ(RewriteParams("SELECT $p1 + 10", params).Sql,
        "DECLARE $p1 AS Int32;\nSELECT $p1 + 10");
}

TEST(OdbcParamRewrite, SkipsParameterMarkersInComments) {
    const std::string sql = "SELECT ? -- optional ? $p8\n/* disabled $p9 ? */";
    const auto result = RewriteParams(sql, {IntParam(1)});
    ASSERT_TRUE(result.Success);
    EXPECT_EQ(result.Sql,
        "DECLARE $p1 AS Int32;\n"
        "SELECT $p1 -- optional ? $p8\n/* disabled $p9 ? */");
    EXPECT_EQ(CountOdbcParams(sql), 1);
    EXPECT_EQ(CountOdbcParams("SELECT 1 -- optional ? $p8"), 0);
    EXPECT_EQ(CountOdbcParams("SELECT 1 /* disabled ? $p9"), 0);
}

TEST(OdbcParamRewrite, PrependsDeclareForNativeDollarParams) {
    const std::vector<TBoundParam> params = {IntParam(1), IntParam(2)};
    const auto result = RewriteParams("SELECT $p1 + $p2 AS result", params);
    ASSERT_TRUE(result.Success);
    EXPECT_EQ(result.Sql,
        "DECLARE $p1 AS Int32;\n"
        "DECLARE $p2 AS Int32;\n"
        "SELECT $p1 + $p2 AS result");
}

TEST(OdbcParamRewrite, DeclaresOnlyNullValuesOptional) {
    const auto result = RewriteParams(
        "SELECT ?, ?", {IntParam(1), NullIntParam(2)});
    ASSERT_TRUE(result.Success);
    EXPECT_EQ(result.Sql,
        "DECLARE $p1 AS Int32;\n"
        "DECLARE $p2 AS Int32?;\n"
        "SELECT $p1, $p2");
}

TEST(OdbcParamRewrite, RejectsMismatchedBindCount) {
    const auto result = RewriteParams("SELECT ? + ?", {IntParam(1)});
    ASSERT_FALSE(result.Success);
    EXPECT_EQ(result.SqlState, "07002");
}

TEST(OdbcParamRewrite, CountOdbcParams) {
    EXPECT_EQ(CountOdbcParams("SELECT ? + ?"), 2);
    EXPECT_EQ(CountOdbcParams("SELECT $p1"), 1);
    EXPECT_EQ(CountOdbcParams("SELECT $p1 + $p2"), 2);
    EXPECT_EQ(CountOdbcParams("SELECT 1"), 0);
}

TEST(OdbcParamRewrite, ClassifiesAfterTrivia) {
    EXPECT_TRUE(StartsWithSqlStatement(" -- lead\n /* block */ INSERT INTO t VALUES (1)", {"INSERT"}));
    EXPECT_FALSE(StartsWithSqlStatement(" -- lead\n SELECT 1", {"INSERT", "UPDATE"}));
}
