#include "utils/bindings.h"
#include "utils/param_rewrite.h"

#include <gtest/gtest.h>

using NYdb::NOdbc::RewriteOdbcSql;
using NYdb::NOdbc::CountOdbcParams;
using NYdb::NOdbc::GetDeclaredParamOptionality;
using NYdb::NOdbc::HasMultipleSqlStatements;
using NYdb::NOdbc::StartsWithSqlStatement;
using NYdb::NOdbc::TBoundParam;
using NYdb::NOdbc::BuildParamMetadataQuery;

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

TEST(OdbcParamRewrite, ParameterMetadataProbes) {
    const std::pair<const char*, const char*> cases[] = {
        {"/* ? */ INSERT INTO `test` (`id`, str) VALUES (?, ?)",
         "SELECT `id` AS __p1, str AS __p2 FROM `test` LIMIT 0"},
        {"SELECT * FROM t AS x WHERE x.id = ? AND x.str = ? ORDER BY id, str",
         "SELECT x.id AS __p1, x.str AS __p2 FROM t AS x LIMIT 0"},
        {"UPDATE t SET value = ?, id = ? WHERE id = ?",
         "SELECT value AS __p1, id AS __p2, id AS __p3 FROM t LIMIT 0"},
        {"SELECT CASE WHEN (? = 1) THEN 1.5 ELSE 2.5 END, (? = 'x'), ? IS NULL",
         "SELECT Just(1) AS __p1, CAST(NULL AS Utf8) AS __p2, NULL AS __p3 LIMIT 0"},
        {"SELECT {fn CONVERT(?, SQL_INTEGER)}, CAST(? AS DECIMAL(7,2))",
         "SELECT CAST(NULL AS Int32) AS __p1, CAST(NULL AS Decimal(7,2)) AS __p2 LIMIT 0"},
        {"DECLARE $p1 AS Int32?; SELECT $p1 = 1",
         "SELECT CAST(NULL AS Int32?) AS __p1 LIMIT 0"},
        {"SELECT (? = -1), (? = Date('2026-10-26')), (? = Utf8('12:34:56'))",
         "SELECT Just(-1) AS __p1, CAST(NULL AS Date) AS __p2, CAST(NULL AS Utf8) AS __p3 LIMIT 0"},
        {"SELECT (? = {d '2026-10-26'}), (? = {ts '2026-10-26 12:34:56'})",
         "SELECT CAST(NULL AS Date) AS __p1, CAST(NULL AS Timestamp) AS __p2 LIMIT 0"},
    };
    for (const auto& [sql, expected] : cases) {
        SCOPED_TRACE(sql);
        EXPECT_EQ(BuildParamMetadataQuery(sql, true), expected);
    }
    for (const char* sql : {"SELECT ?", "SELECT (? = DangerousFunction())",
            "SELECT * FROM t WHERE ? = DangerousFunction()",
            "SELECT (? = 1 + 2)", "SELECT (? = 1); DELETE FROM t", "SELECT * FROM t, u WHERE t.id = ?",
            "SELECT * FROM t JOIN u ON t.id = u.id WHERE t.id = ?"}) {
        SCOPED_TRACE(sql);
        EXPECT_FALSE(BuildParamMetadataQuery(sql, true));
    }
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

TEST(OdbcParamRewrite, DetectsOptionalDeclarationsBeforeComments) {
    EXPECT_EQ(GetDeclaredParamOptionality(
        "DECLARE $p1 AS Int32? /* nullable */;\nSELECT $p1", 1), true);
    EXPECT_EQ(GetDeclaredParamOptionality(
        "DECLARE $p1 AS Int32? -- nullable\n;\nSELECT $p1", 1), true);
    EXPECT_EQ(GetDeclaredParamOptionality(
        "DECLARE $p1 AS Int32 /* not optional?; */;\nSELECT $p1", 1), false);
    EXPECT_EQ(GetDeclaredParamOptionality(
        "/* DECLARE $p1 AS Int32?; */\nSELECT $p1", 1), std::nullopt);
    EXPECT_EQ(GetDeclaredParamOptionality(
        "pragma TablePathPrefix = \"/local\"; declare /* p */ $p1 as Int32?; SELECT $p1", 1),
        true);
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
    EXPECT_TRUE(StartsWithSqlStatement(
        "PRAGMA TablePathPrefix = \"/local\"; DECLARE $p1 AS Int32; UPSERT INTO t VALUES ($p1)",
        {"UPSERT"}));
    EXPECT_TRUE(StartsWithSqlStatement(
        "$rows = (SELECT 1); SELECT * FROM $rows; -- trailing", {"SELECT"}));
    EXPECT_TRUE(StartsWithSqlStatement(
        "DEFINE ACTION $read() AS SELECT 1; END DEFINE; SELECT 2", {"SELECT"}));
    EXPECT_TRUE(StartsWithSqlStatement(
        "PRAGMA config = \"quoted\\\";value\"; CREATE TABLE t (id Int32)", {"CREATE"}));
    EXPECT_TRUE(StartsWithSqlStatement("DELETE FROM t; SELECT * FROM t", {"DELETE"}));
    EXPECT_FALSE(StartsWithSqlStatement("DELETE FROM t; SELECT * FROM t", {"SELECT"}));
    EXPECT_TRUE(HasMultipleSqlStatements("DELETE FROM t; SELECT * FROM t"));
    EXPECT_TRUE(HasMultipleSqlStatements("PRAGMA x; DELETE FROM t; SELECT * FROM t"));
    EXPECT_FALSE(HasMultipleSqlStatements("PRAGMA x; SELECT 1; -- trailing"));
    EXPECT_FALSE(HasMultipleSqlStatements(
        "DEFINE ACTION $read() AS SELECT 1; END DEFINE; SELECT 2"));
    EXPECT_FALSE(StartsWithSqlStatement(" -- lead\n SELECT 1", {"INSERT", "UPDATE"}));
}
