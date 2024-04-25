#include "env.h"

#include "env_var.h"

#include <src/util/folder/dirut.h>
#include <src/util/folder/path.h>
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <src/util/stream/file.h>
#include <ydb-cpp-sdk/util/stream/fwd.h>
#include <ydb-cpp-sdk/util/system/file.h>
#include <src/util/system/file_lock.h>
#include <ydb-cpp-sdk/util/system/guard.h>

#include <ydb-cpp-sdk/library/json/json_reader.h>
#include <ydb-cpp-sdk/library/json/json_value.h>
<<<<<<< HEAD
=======
#include <src/util/generic/singleton.h>
#include <src/util/stream/file.h>
#include <src/util/stream/fwd.h>
#include <src/util/system/file.h>
#include <src/util/system/file_lock.h>
#include <src/util/system/guard.h>

#include <src/library/json/json_reader.h>
#include <src/library/json/json_value.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
#include <src/library/json/json_writer.h>
#include <src/library/svnversion/svnversion.h>

#include <mutex>

std::string ArcadiaSourceRoot() {
    if (const auto& sourceRoot = NPrivate::GetTestEnv().SourceRoot; !sourceRoot.empty()) {
        return sourceRoot;
    } else {
        return GetArcadiaSourcePath();
    }
}

std::string BuildRoot() {
    if (const auto& buildRoot = NPrivate::GetTestEnv().BuildRoot; !buildRoot.empty()) {
        return buildRoot;
    } else {
        return GetArcadiaSourcePath();
    }
}

std::string ArcadiaFromCurrentLocation(std::string_view where, std::string_view path) {
    return (TFsPath(ArcadiaSourceRoot()) / TFsPath(where).Parent() / path).Fix();
}

std::string BinaryPath(std::string_view path) {
    return (TFsPath(BuildRoot()) / path).Fix();
}

std::string GetArcadiaTestsData() {
    std::string atdRoot = NPrivate::GetTestEnv().ArcadiaTestsDataDir;
    if (!atdRoot.empty()) {
        return atdRoot;
    }

    std::string path = NPrivate::GetCwd();
    const char pathsep = GetDirectorySeparator();
    while (!path.empty()) {
        std::string dataDir = path + "/arcadia_tests_data";
        if (IsDir(dataDir)) {
            return dataDir;
        }

        auto pos = path.find_last_of(pathsep);
        if (pos == std::string::npos) {
            pos = 0;
        }
        path.erase(pos);
    }

    return {};
}

std::string GetWorkPath() {
    std::string workPath = NPrivate::GetTestEnv().WorkPath;
    if (!workPath.empty()) {
        return workPath;
    }

    return NPrivate::GetCwd();
}

TFsPath GetOutputPath() {
    return GetWorkPath() + "/testing_out_stuff";
}

const std::string& GetRamDrivePath() {
    return NPrivate::GetTestEnv().RamDrivePath;
}

const std::string& GetYtHddPath() {
    return NPrivate::GetTestEnv().YtHddPath;
}

const std::string& GetOutputRamDrivePath() {
    return NPrivate::GetTestEnv().TestOutputRamDrivePath;
}

const std::string& GdbPath() {
    return NPrivate::GetTestEnv().GdbPath;
}

const std::string& GetTestParam(std::string_view name) {
    const static std::string def = "";
    return GetTestParam(name, def);
}

const std::string& GetTestParam(std::string_view name, const std::string& def) {
    auto& testParameters = NPrivate::GetTestEnv().TestParameters;
    auto it = testParameters.find(name.data());
    if (it != testParameters.end()) {
        return it->second;
    }
    return def;
}

const std::string& GetGlobalResource(std::string_view name) {
    auto& resources = NPrivate::GetTestEnv().GlobalResources;
    auto it = resources.find(name.data());
    Y_ABORT_UNLESS(it != resources.end());
    return it->second;
}

void AddEntryToCoreSearchFile(const std::string& filename, std::string_view cmd, int pid, const TFsPath& binaryPath = TFsPath(), const TFsPath& cwd = TFsPath()) {
    auto lock = TFileLock(filename);
    std::lock_guard guard(lock);

    TOFStream output(TFile(filename, WrOnly | ForAppend | OpenAlways));

    NJson::TJsonWriter writer(&output, false);
    writer.OpenMap();
    writer.Write("cmd", cmd);
    writer.Write("pid", pid);
    if (binaryPath) {
        writer.Write("binary_path", binaryPath);
    }
    if (cwd) {
        writer.Write("cwd", cwd);
    }
    writer.CloseMap();
    writer.Flush();

    output.Write("\n");
}

void WatchProcessCore(int pid, const TFsPath& binaryPath, const TFsPath& cwd) {
    auto& filename = NPrivate::GetTestEnv().CoreSearchFile;
    if (!filename.empty()) {
        AddEntryToCoreSearchFile(filename, "add", pid, binaryPath, cwd);
    }
}

void StopProcessCoreWatching(int pid) {
    auto& filename = NPrivate::GetTestEnv().CoreSearchFile;
    if (!filename.empty()) {
        AddEntryToCoreSearchFile(filename, "drop", pid);
    }
}

bool FromYaTest() {
    return NPrivate::GetTestEnv().IsRunningFromTest;
}

namespace NPrivate {
    TTestEnv::TTestEnv() {
        ReInitialize();
    }

    void TTestEnv::ReInitialize() {
        IsRunningFromTest = false;
        ArcadiaTestsDataDir = "";
        SourceRoot = "";
        BuildRoot = "";
        WorkPath = "";
        RamDrivePath = "";
        YtHddPath = "";
        TestOutputRamDrivePath = "";
        GdbPath = "";
        CoreSearchFile = "";
        EnvFile = "";
        TestParameters.clear();
        GlobalResources.clear();

        const std::string contextFilename = NUtils::GetEnv("YA_TEST_CONTEXT_FILE");
        if (!contextFilename.empty() && TFsPath(contextFilename).Exists()) {
            NJson::TJsonValue context;
            NJson::ReadJsonTree(TFileInput(contextFilename).ReadAll(), &context);

            NJson::TJsonValue* value;

            value = context.GetValueByPath("runtime.source_root");
            if (value) {
                SourceRoot = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.build_root");
            if (value) {
                BuildRoot = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.atd_root");
            if (value) {
                ArcadiaTestsDataDir = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.work_path");
            if (value) {
                WorkPath = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.ram_drive_path");
            if (value) {
                RamDrivePath = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.yt_hdd_path");
            if (value) {
                YtHddPath = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.test_output_ram_drive_path");
            if (value) {
                TestOutputRamDrivePath = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.gdb_bin");
            if (value) {
                GdbPath = value->GetStringSafe("");
            }

            value = context.GetValueByPath("runtime.test_params");
            if (value) {
                for (const auto& entry : context.GetValueByPath("runtime.test_params")->GetMap()) {
                    TestParameters[entry.first] = entry.second.GetStringSafe("");
                }
            }

            value = context.GetValueByPath("resources.global");
            if (value) {
                for (const auto& entry : value->GetMap()) {
                    GlobalResources[entry.first] = entry.second.GetStringSafe("");
                }
            }

            value = context.GetValueByPath("internal.core_search_file");
            if (value) {
                CoreSearchFile = value->GetStringSafe("");
            }

            value = context.GetValueByPath("internal.env_file");
            if (value) {
                EnvFile = value->GetStringSafe("");
                if (TFsPath(EnvFile).Exists()) {
                    TFileInput file(EnvFile);
                    NJson::TJsonValue envVar;
                    std::string ljson;
                    while (file.ReadLine(ljson) > 0) {
                        NJson::ReadJsonTree(ljson, &envVar);
                        for (const auto& entry : envVar.GetMap()) {
                            NUtils::SetEnv(entry.first, entry.second.GetStringSafe(""));
                        }
                    }
                }
            }
        }

        if (YtHddPath.empty()) {
            YtHddPath = NUtils::GetEnv("HDD_PATH");
        }

        if (SourceRoot.empty()) {
            SourceRoot = NUtils::GetEnv("ARCADIA_SOURCE_ROOT");
        }

        if (BuildRoot.empty()) {
            BuildRoot = NUtils::GetEnv("ARCADIA_BUILD_ROOT");
        }

        if (ArcadiaTestsDataDir.empty()) {
            ArcadiaTestsDataDir = NUtils::GetEnv("ARCADIA_TESTS_DATA_DIR");
        }

        if (WorkPath.empty()) {
            WorkPath = NUtils::GetEnv("TEST_WORK_PATH");
        }

        if (RamDrivePath.empty()) {
            RamDrivePath = NUtils::GetEnv("YA_TEST_RAM_DRIVE_PATH");
        }

        if (TestOutputRamDrivePath.empty()) {
            TestOutputRamDrivePath = NUtils::GetEnv("YA_TEST_OUTPUT_RAM_DRIVE_PATH");
        }

        const std::string fromEnv = NUtils::GetEnv("YA_TEST_RUNNER");
        IsRunningFromTest = (fromEnv == "1");
    }

    void TTestEnv::AddTestParam(std::string_view name, std::string_view value) {
        TestParameters[std::string{name}] = value;
    }

    std::string GetCwd() {
        try {
            return NFs::CurrentWorkingDirectory();
        } catch (...) {
            return {};
        }
    }

    const TTestEnv& GetTestEnv() {
        return *Singleton<TTestEnv>();
    }
}
