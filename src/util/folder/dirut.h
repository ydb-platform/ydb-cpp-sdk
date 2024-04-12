#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/system/sysstat.h>
#include <src/util/system/fs.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/system/defaults.h>
#include <src/util/system/sysstat.h>
#include <src/util/system/fs.h>
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <sys/types.h>

#include <cerrno>
#include <cstdlib>

#ifdef _win32_
<<<<<<< HEAD
    #include <ydb-cpp-sdk/util/system/winint.h>
=======
    #include <src/util/system/winint.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
    #include <direct.h>
    #include <malloc.h>
    #include <time.h>
    #include <io.h>
    #include "dirent_win.h"

// these live in mktemp_system.cpp
extern "C" int mkstemps(char* path, int slen);
char* mkdtemp(char* path);

#else
    #ifdef _sun_
        #include <alloca.h>

char* mkdtemp(char* path);
    #endif
    #include <unistd.h>
    #include <pwd.h>
    #include <dirent.h>
    #ifndef DT_DIR
        #include <sys/stat.h>
    #endif
#endif

bool IsDir(const std::string& path);

int mkpath(char* path, int mode = 0777);

std::string GetHomeDir();

void MakeDirIfNotExist(const char* path, int mode = 0777);

inline void MakeDirIfNotExist(const std::string& path, int mode = 0777) {
    MakeDirIfNotExist(path.data(), mode);
}

/// Create path making parent directories as needed
void MakePathIfNotExist(const char* path, int mode = 0777);

void SlashFolderLocal(std::string& folder);
bool correctpath(std::string& filename);
bool resolvepath(std::string& folder, const std::string& home);

char GetDirectorySeparator();
const char* GetDirectorySeparatorS();

void RemoveDirWithContents(std::string dirName);

const char* GetFileNameComponent(const char* f);

inline std::string GetFileNameComponent(const std::string& f) {
    return GetFileNameComponent(f.data());
}

/// RealPath doesn't guarantee trailing separator to be stripped or left in place for directories.
std::string RealPath(const std::string& path);     // throws
std::string RealLocation(const std::string& path); /// throws; last file name component doesn't need to exist

std::string GetSystemTempDir();

int MakeTempDir(char path[/*FILENAME_MAX*/], const char* prefix);

int ResolvePath(const char* rel, const char* abs, char res[/*FILENAME_MAX*/], bool isdir = false);
std::string ResolvePath(const char* rel, const char* abs, bool isdir = false);
std::string ResolvePath(const char* path, bool isDir = false);

std::string ResolveDir(const char* path);

bool SafeResolveDir(const char* path, std::string& result);

std::string GetDirName(const std::string& path);

std::string GetBaseName(const std::string& path);

std::string StripFileComponent(const std::string& fileName);

class TExistenceChecker {
public:
    TExistenceChecker(bool strict = false)
        : Strict(strict)
    {
    }

    void SetStrict(bool strict) {
        Strict = strict;
    }

    bool IsStrict() const {
        return Strict;
    }

    const char* Check(const char* fname) const {
        if (!fname || !*fname)
            return nullptr;
        if (Strict) {
            NFs::EnsureExists(fname);
        } else if (!NFs::Exists(fname))
            fname = nullptr;
        return fname;
    }

private:
    bool Strict;
};
