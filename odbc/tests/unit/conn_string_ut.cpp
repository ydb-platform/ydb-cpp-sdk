#include "utils/util.h"

#include <gtest/gtest.h>

using NYdb::NOdbc::ParseConnectionStringEntries;
using NYdb::NOdbc::TConnectionStringEntries;

TEST(ConnString, ParsesBraceEscapedSemicolons) {
    EXPECT_EQ(ParseConnectionStringEntries("Database={path;with;semicolons};Server=host"),
        (TConnectionStringEntries{{"Database", "path;with;semicolons"}, {"Server", "host"}}));
}

TEST(ConnString, DecodesEscapedClosingBrace) {
    EXPECT_EQ(ParseConnectionStringEntries("PWD={a}}b};Database=/local;Server=host"),
        (TConnectionStringEntries{{"PWD", "a}b"}, {"Database", "/local"}, {"Server", "host"}}));
}

TEST(ConnString, ParsesSimplePairs) {
    EXPECT_EQ(ParseConnectionStringEntries("DSN=YDB;Database=/local;Server=grpc://localhost:2136"),
        (TConnectionStringEntries{{"DSN", "YDB"}, {"Database", "/local"}, {"Server", "grpc://localhost:2136"}}));
}

TEST(ConnString, TrimsWhitespace) {
    EXPECT_EQ(ParseConnectionStringEntries(" Database = /local ; Server = host "),
        (TConnectionStringEntries{{"Database", "/local"}, {"Server", "host"}}));
}

TEST(OdbcString, ConvertsUtf16ToUtf8) {
    SQLWCHAR text[] = {'Y', 'D', 'B', ' ', 0x041f, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442, 0};
    EXPECT_EQ(NYdb::NOdbc::GetString(text, SQL_NTS), "YDB \xd0\x9f\xd1\x80\xd0\xb8\xd0\xb2\xd0\xb5\xd1\x82");
}

TEST(OdbcString, ConvertsUtf16SurrogatePairToUtf8) {
    SQLWCHAR text[] = {0xd83d, 0xde80, 0};
    EXPECT_EQ(NYdb::NOdbc::GetString(text, SQL_NTS), "\xf0\x9f\x9a\x80");
}

TEST(OdbcString, IgnoresAnsiTerminatorIncludedInExplicitLength) {
    SQLCHAR text[] = {'/', 'l', 'o', 'c', 'a', 'l', 0};
    EXPECT_EQ(NYdb::NOdbc::GetString(text, 7), "/local");
}

TEST(OdbcString, IgnoresUtf16TerminatorIncludedInExplicitLength) {
    SQLWCHAR text[] = {'S', 'E', 'L', 'E', 'C', 'T', ' ', '4', '2', 0};
    EXPECT_EQ(NYdb::NOdbc::GetString(text, 10), "SELECT 42");
}
