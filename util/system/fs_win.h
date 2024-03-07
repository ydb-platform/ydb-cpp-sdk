#pragma once

#include "winint.h"
#include "defaults.h"


namespace NFsPrivate {
    bool WinRename(const std::string& oldPath, const std::string& newPath);

    bool WinSymLink(const std::string& targetName, const std::string& linkName);

    bool WinHardLink(const std::string& existingPath, const std::string& newPath);

    std::string WinReadLink(const std::string& path);

    ULONG WinReadReparseTag(HANDLE h);

    HANDLE CreateFileWithUtf8Name(const std::string_view fName, ui32 accessMode, ui32 shareMode, ui32 createMode, ui32 attributes, bool inheritHandle);

    bool WinRemove(const std::string& path);

    bool WinExists(const std::string& path);

    std::string WinCurrentWorkingDirectory();

    bool WinSetCurrentWorkingDirectory(const std::string& path);

    bool WinMakeDirectory(const std::string& path);
}
