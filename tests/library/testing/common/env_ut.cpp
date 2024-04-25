#include <src/library/testing/common/env.h>
#include <src/library/testing/common/scope.h>
#include <src/library/testing/gtest/gtest.h>

#include <src/util/folder/dirut.h>
#include <src/util/stream/file.h>
#include <src/util/system/execpath.h>
#include <src/util/system/fs.h>


TEST(Runtime, ArcadiaSourceRoot) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    {
        auto tmpDir = ::GetSystemTempDir();
        NTesting::TScopedEnvironment guard("ARCADIA_SOURCE_ROOT", tmpDir);
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        EXPECT_EQ(tmpDir, ArcadiaSourceRoot());
    }
    {
        NTesting::TScopedEnvironment guard("ARCADIA_SOURCE_ROOT", "");
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        EXPECT_FALSE(ArcadiaSourceRoot().empty());
    }
}

TEST(Runtime, BuildRoot) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    {
        auto tmpDir = ::GetSystemTempDir();
        NTesting::TScopedEnvironment guard("ARCADIA_BUILD_ROOT", tmpDir);
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        EXPECT_EQ(tmpDir, BuildRoot());
    }
    {
        NTesting::TScopedEnvironment guard("ARCADIA_BUILD_ROOT", "");
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        EXPECT_FALSE(BuildRoot().empty());
    }
}

TEST(Runtime, BinaryPath) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    Singleton<NPrivate::TTestEnv>()->ReInitialize();
<<<<<<< HEAD
=======
<<<<<<<< HEAD:src/library/testing/common/ut/env_ut.cpp
>>>>>>> origin/main
<<<<<<<< HEAD:tests/library/testing/common/env_ut.cpp
    EXPECT_TRUE(TFsPath(BinaryPath("tests/library/testing/common")).Exists());
========
    EXPECT_TRUE(TFsPath(BinaryPath("src/library/testing/common/ut")).Exists());
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/testing/common/ut/env_ut.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/testing/common/ut/env_ut.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD
=======
========
    EXPECT_TRUE(TFsPath(BinaryPath("tests/library/testing/common")).Exists());
>>>>>>>> origin/main:tests/library/testing/common/env_ut.cpp
>>>>>>> origin/main
}

TEST(Runtime, GetArcadiaTestsData) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    {
        auto tmpDir = ::GetSystemTempDir();
        NTesting::TScopedEnvironment guard("ARCADIA_TESTS_DATA_DIR", tmpDir);
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        EXPECT_EQ(tmpDir, GetArcadiaTestsData());
    }
    {
        NTesting::TScopedEnvironment guard("ARCADIA_TESTS_DATA_DIR", "");
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        auto path = GetArcadiaTestsData();
        // it is not error if path is empty
        const bool ok = (path.empty() || GetBaseName(path) == "arcadia_tests_data");
        EXPECT_TRUE(ok);
    }
}

TEST(Runtime, GetWorkPath) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    {
        auto tmpDir = ::GetSystemTempDir();
        NTesting::TScopedEnvironment guard("TEST_WORK_PATH", tmpDir);
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        EXPECT_EQ(tmpDir, GetWorkPath());
    }
    {
        NTesting::TScopedEnvironment guard("TEST_WORK_PATH", "");
        Singleton<NPrivate::TTestEnv>()->ReInitialize();
        EXPECT_TRUE(!GetWorkPath().empty());
    }
}

TEST(Runtime, GetOutputPath) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    Singleton<NPrivate::TTestEnv>()->ReInitialize();
    EXPECT_EQ(GetOutputPath().Basename(), "testing_out_stuff");
}

TEST(Runtime, GetRamDrivePath) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    auto tmpDir = ::GetSystemTempDir();
    NTesting::TScopedEnvironment guard("YA_TEST_RAM_DRIVE_PATH", tmpDir);
    Singleton<NPrivate::TTestEnv>()->ReInitialize();
    EXPECT_EQ(tmpDir, GetRamDrivePath());
}

TEST(Runtime, GetOutputRamDrivePath) {
    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", ""); // remove context filename
    auto tmpDir = ::GetSystemTempDir();
    NTesting::TScopedEnvironment guard("YA_TEST_OUTPUT_RAM_DRIVE_PATH", tmpDir);
    Singleton<NPrivate::TTestEnv>()->ReInitialize();
    EXPECT_EQ(tmpDir, GetOutputRamDrivePath());
}

std::string ReInitializeContext(std::string_view data) {
    auto tmpDir = ::GetSystemTempDir();
    auto filename = tmpDir + "/context.json";
    TOFStream stream(filename);
    stream.Write(data.data(), data.size());
    stream.Finish();

    NTesting::TScopedEnvironment contextGuard("YA_TEST_CONTEXT_FILE", filename);
    Singleton<NPrivate::TTestEnv>()->ReInitialize();

    return filename;
}

TEST(Runtime, GetTestParam) {
    std::string context = R"json({
        "runtime": {
            "test_params": {
                "a": "b",
                "c": "d"
            }
        }
    })json";
    auto filename = ReInitializeContext(context);

    EXPECT_EQ("b", GetTestParam("a"));
    EXPECT_EQ("d", GetTestParam("c"));
    EXPECT_EQ("", GetTestParam("e"));
    EXPECT_EQ("w", GetTestParam("e", "w"));

    Singleton<NPrivate::TTestEnv>()->AddTestParam("e", "e");
    EXPECT_EQ("e", GetTestParam("e"));
}

TEST(Runtime, WatchProcessCore) {
    std::string context = R"json({
        "internal": {
            "core_search_file": "watch_core.txt"
        }
    })json";
    auto filename = ReInitializeContext(context);

    WatchProcessCore(1, "bin1", "pwd");
    WatchProcessCore(2, "bin1");
    StopProcessCoreWatching(2);

    TIFStream file("watch_core.txt");
    auto data = file.ReadAll();
    TFsPath("watch_core.txt").DeleteIfExists();
    std::string expected = R"json({"cmd":"add","pid":1,"binary_path":"bin1","cwd":"pwd"}
{"cmd":"add","pid":2,"binary_path":"bin1"}
{"cmd":"drop","pid":2}
)json";
    EXPECT_EQ(expected, data);
}

TEST(Runtime, GlobalResources) {
    std::string context = R"json({
        "resources": {
            "global": {
                "TOOL_NAME_RESOURCE_GLOBAL": "path"
            }
        }
    })json";

    auto filename = ReInitializeContext(context);
    EXPECT_EQ("path", GetGlobalResource("TOOL_NAME_RESOURCE_GLOBAL"));
}
