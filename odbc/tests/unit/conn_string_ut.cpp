#include "utils/util.h"

#include <gtest/gtest.h>

TEST(ConnString, ParsesBraceEscapedSemicolons) {
    const auto params = NYdb::NOdbc::ParseConnectionString("Database={path;with;semicolons};Server=host");
    ASSERT_EQ(params.at("Database"), "path;with;semicolons");
    ASSERT_EQ(params.at("Server"), "host");
}

TEST(ConnString, ParsesSimplePairs) {
    const auto params = NYdb::NOdbc::ParseConnectionString("DSN=YDB;Database=/local;Server=grpc://localhost:2136");
    ASSERT_EQ(params.at("DSN"), "YDB");
    ASSERT_EQ(params.at("Database"), "/local");
    ASSERT_EQ(params.at("Server"), "grpc://localhost:2136");
}

TEST(ConnString, TrimsWhitespace) {
    const auto params = NYdb::NOdbc::ParseConnectionString(" Database = /local ; Server = host ");
    ASSERT_EQ(params.at("Database"), "/local");
    ASSERT_EQ(params.at("Server"), "host");
}
