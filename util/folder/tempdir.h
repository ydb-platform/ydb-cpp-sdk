#pragma once

#include "fwd.h"
#include "path.h"

class TTempDir {
public:
    /// Create new directory in system tmp folder.
    TTempDir();

    /// Create new directory with this fixed name. If it already exists, clear it.
    TTempDir(const std::string& tempDir);

    ~TTempDir();

    /// Create new directory in given folder.
    static TTempDir NewTempDir(const std::string& root);

    const std::string& operator()() const {
        return Name();
    }

    const std::string& Name() const {
        return TempDir.GetPath();
    }

    const TFsPath& Path() const {
        return TempDir;
    }

    void DoNotRemove();

private:
    struct TCreationToken {};

    // Prevent people from confusing this ctor with the public one
    // by requiring additional fake argument.
    TTempDir(const char* prefix, TCreationToken);

    TFsPath TempDir;
    bool Remove;
};
