#include <library/cpp/testing/common/env_var.h>
#include <library/cpp/testing/common/scope.h>

#include <library/cpp/testing/gtest/gtest.h>

TEST(TScopedEnvironment, SingleValue) {
    const std::string before = NUtils::GetEnv("ARCADIA_SOURCE_ROOT");
    {
        NTesting::TScopedEnvironment guard("ARCADIA_SOURCE_ROOT", "source");
        EXPECT_EQ("source", NUtils::GetEnv("ARCADIA_SOURCE_ROOT"));
    }
    EXPECT_EQ(before, NUtils::GetEnv("ARCADIA_SOURCE_ROOT"));
}

TEST(TScopedEnvironment, MultiValue) {
    const std::vector<std::string> before{NUtils::GetEnv("ARCADIA_SOURCE_ROOT"), NUtils::GetEnv("ARCADIA_BUILD_ROOT")};
    {
        NTesting::TScopedEnvironment guard{{
            {"ARCADIA_SOURCE_ROOT", "source"},
            {"ARCADIA_BUILD_ROOT", "build"},
        }};
        EXPECT_EQ("source", NUtils::GetEnv("ARCADIA_SOURCE_ROOT"));
        EXPECT_EQ("build", NUtils::GetEnv("ARCADIA_BUILD_ROOT"));
    }
    const std::vector<std::string> after{NUtils::GetEnv("ARCADIA_SOURCE_ROOT"), NUtils::GetEnv("ARCADIA_BUILD_ROOT")};
    EXPECT_EQ(before, after);
}
