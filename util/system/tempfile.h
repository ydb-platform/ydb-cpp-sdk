#pragma once

#include "fs.h"
#include "file.h"

#include <util/folder/path.h>

class TTempFile {
public:
    inline TTempFile(const std::string& fname)
        : Name_(fname)
    {
    }

    inline ~TTempFile() {
        NFs::Remove(Name());
    }

    inline const std::string& Name() const noexcept {
        return Name_;
    }

private:
    const std::string Name_;
};

class TTempFileHandle: public TTempFile, public TFile {
public:
    TTempFileHandle();
    TTempFileHandle(const std::string& fname);

    static TTempFileHandle InCurrentDir(const std::string& filePrefix = "yandex", const std::string& extension = "tmp");
    static TTempFileHandle InDir(const TFsPath& dirPath, const std::string& filePrefix = "yandex", const std::string& extension = "tmp");

private:
    TFile CreateFile() const;
};

/*
 * Creates a unique temporary filename in specified directory.
 * If specified directory is NULL or empty, then system temporary directory is used.
 *
 * Note, that the function is not race-free, the file is guaranteed to exist at the time the function returns, but not at the time the returned name is first used.
 * Throws TSystemError on error.
 *
 * Returned filepath has such format: dir/prefixXXXXXX.extension or dir/prefixXXXXXX
 * But win32: dir/preXXXX.tmp (prefix is up to 3 characters, extension is always tmp).
 */
std::string MakeTempName(const char* wrkDir = nullptr, const char* prefix = "yandex", const char* extension = "tmp");
