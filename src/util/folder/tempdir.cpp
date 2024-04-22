#include "tempdir.h"

#include "dirut.h"

#include <src/util/system/fs.h>
#include <ydb-cpp-sdk/util/system/maxlen.h>

TTempDir::TTempDir()
    : TTempDir(nullptr, TCreationToken{})
{
}

TTempDir::TTempDir(const char* prefix, TCreationToken)
    : TempDir()
    , Remove(true)
{
    char tempDir[MAX_PATH];
    if (MakeTempDir(tempDir, prefix) != 0) {
        ythrow TSystemError() << "Can't create temporary directory";
    }
    TempDir = tempDir;
}

TTempDir::TTempDir(const std::string& tempDir)
    : TempDir(tempDir)
    , Remove(true)
{
    NFs::Remove(TempDir);
    MakeDirIfNotExist(TempDir.c_str());
}

TTempDir TTempDir::NewTempDir(const std::string& root) {
    return {root.c_str(), TCreationToken{}};
}

void TTempDir::DoNotRemove() {
    Remove = false;
}

TTempDir::~TTempDir() {
    if (Remove) {
        RemoveDirWithContents(TempDir);
    }
}
