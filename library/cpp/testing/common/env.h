#pragma once

#include <unordered_map>

#include <util/folder/path.h>
#include <util/generic/string.h>
#include <util/generic/strbuf.h>
#include <util/system/src_location.h>

// @brief return full path to arcadia root
std::string ArcadiaSourceRoot();

// @brief return full path for file or folder specified by known source location `where` and `path` which is relative to parent folder of `where`
//        for the instance: there is 2 files in folder test example_ut.cpp and example.data, so full path to test/example.data can be obtained
//        from example_ut.cpp as ArcadiaFromCurrentLocation(__SOURCE_FILE__, "example.data")
std::string ArcadiaFromCurrentLocation(std::string_view where, std::string_view path);

// @brief return build folder path
std::string BuildRoot();

// @brief return full path to built artefact, where path is relative from arcadia root
std::string BinaryPath(std::string_view path);

// @brief return true if environment is testenv otherwise false
bool FromYaTest();

// @brief returns TestsData dir (from env:ARCADIA_TESTS_DATA_DIR or path to existing folder `arcadia_tests_data` within parent folders)
std::string GetArcadiaTestsData();

// @brief return current working dir (from env:TEST_WORK_PATH or cwd)
std::string GetWorkPath();

// @brief return tests output path (workdir + testing_out_stuff)
TFsPath GetOutputPath();

// @brief return path from env:YA_TEST_RAM_DRIVE_PATH
const std::string& GetRamDrivePath();

// @brief return path from env:YA_TEST_OUTPUT_RAM_DRIVE_PATH
const std::string& GetOutputRamDrivePath();

// @brief return test parameter by name. If not exists, return an empty string
const std::string& GetTestParam(std::string_view name);

// @brief return test parameter by name. If not exists, return specified default value
const std::string& GetTestParam(std::string_view name, const std::string& def);

// @brief return path to global resource. If not exists, aborts the test run
const std::string& GetGlobalResource(std::string_view name);

// @brief return path to the gdb
const std::string& GdbPath();

// @brief register the process. Test suite will be marked as failed if the process is terminated with a core dump file after testing
void WatchProcessCore(int pid, const TFsPath& binaryPath, const TFsPath& cwd = TFsPath());

// @brief mark the process as successfully completed - a test machinery won't try to recover core dump file for the process
void StopProcessCoreWatching(int pid);

#define SRC_(path) ArcadiaFromCurrentLocation(__SOURCE_FILE__, path)

namespace NPrivate {
    class TTestEnv {
    public:
        TTestEnv();

        void ReInitialize();

        void AddTestParam(std::string_view name, std::string_view value);

        bool IsRunningFromTest;
        std::string ArcadiaTestsDataDir;
        std::string SourceRoot;
        std::string BuildRoot;
        std::string WorkPath;
        std::string RamDrivePath;
        std::string YtHddPath;
        std::string TestOutputRamDrivePath;
        std::string GdbPath;
        std::string CoreSearchFile;
        std::string EnvFile;
        std::unordered_map<std::string, std::string> TestParameters;
        std::unordered_map<std::string, std::string> GlobalResources;
    };

    std::string GetCwd();

    const TTestEnv& GetTestEnv();
}
