#include "fs.h"
#include "defaults.h"

#if defined(_win_)
    #include "fs_win.h"
#else
    #include <unistd.h>
    #include <errno.h>
#endif

#include <util/string/escape.h>
#include <util/generic/yexception.h>
#include <util/memory/tempbuf.h>
#include <util/stream/file.h>
#include <util/folder/iterator.h>
#include <util/system/fstat.h>
#include <util/folder/path.h>

bool NFs::Remove(const std::string& path) {
#if defined(_win_)
    return NFsPrivate::WinRemove(path);
#else
    return ::remove(path.data()) == 0;
#endif
}

void NFs::RemoveRecursive(const std::string& path) {
    static const std::string_view errStr = "error while removing ";

    if (!NFs::Exists(path)) {
        return;
    }

    if (!TFileStat(path).IsDir()) {
        if (!NFs::Remove(path)) {
            ythrow TSystemError() << errStr << path << " with cwd (" << NFs::CurrentWorkingDirectory() << ")";
        }
    }

    TDirIterator dir(path);

    for (auto it = dir.begin(); it != dir.end(); ++it) {
        switch (it->fts_info) {
            case FTS_DOT:
            case FTS_D:
                break;
            default:
                if (!NFs::Remove(it->fts_path)) {
                    ythrow TSystemError() << errStr << it->fts_path << " with cwd (" << NFs::CurrentWorkingDirectory() << ")";
                }
                break;
        }
    }
}

bool NFs::MakeDirectory(const std::string& path, EFilePermissions mode) {
#if defined(_win_)
    Y_UNUSED(mode);
    return NFsPrivate::WinMakeDirectory(path);
#else
    return mkdir(path.data(), mode) == 0;
#endif
}

bool NFs::MakeDirectoryRecursive(const std::string& path, EFilePermissions mode, bool alwaysCreate) {
    if (NFs::Exists(path) && TFileStat(path).IsDir()) {
        if (alwaysCreate) {
            ythrow TIoException() << "path " << path << " already exists"
                                  << " with cwd (" << NFs::CurrentWorkingDirectory() << ")";
        }
        return true;
    } else {
        //NOTE: recursion is finite due to existence of "." and "/"
        if (!NFs::MakeDirectoryRecursive(TFsPath(path).Parent(), mode, false)) {
            return false;
        }

        bool isDirMade = NFs::MakeDirectory(path, mode);
        if (!isDirMade && alwaysCreate) {
            ythrow TIoException() << "failed to create " << path << " with cwd (" << NFs::CurrentWorkingDirectory() << ")";
        }

        return TFileStat(path).IsDir();
    }
}

bool NFs::Rename(const std::string& oldPath, const std::string& newPath) {
#if defined(_win_)
    return NFsPrivate::WinRename(oldPath, newPath);
#else
    return ::rename(oldPath.data(), newPath.data()) == 0;
#endif
}

void NFs::HardLinkOrCopy(const std::string& existingPath, const std::string& newPath) {
    if (!NFs::HardLink(existingPath, newPath)) {
        Copy(existingPath, newPath);
    }
}

bool NFs::HardLink(const std::string& existingPath, const std::string& newPath) {
#if defined(_win_)
    return NFsPrivate::WinHardLink(existingPath, newPath);
#elif defined(_unix_)
    return (0 == link(existingPath.data(), newPath.data()));
#endif
}

bool NFs::SymLink(const std::string& targetPath, const std::string& linkPath) {
#if defined(_win_)
    return NFsPrivate::WinSymLink(targetPath, linkPath);
#elif defined(_unix_)
    return 0 == symlink(targetPath.data(), linkPath.data());
#endif
}

std::string NFs::ReadLink(const std::string& path) {
#if defined(_win_)
    return NFsPrivate::WinReadLink(path);
#elif defined(_unix_)
    TTempBuf buf;
    while (true) {
        ssize_t r = readlink(path.data(), buf.Data(), buf.Size());
        if (r < 0) {
            ythrow yexception() << "can't read link " << path << ", errno = " << errno;
        }
        if (r < (ssize_t)buf.Size()) {
            return std::string(buf.Data(), r);
        }
        buf = TTempBuf(buf.Size() * 2);
    }
#endif
}

void NFs::Cat(const std::string& dstPath, const std::string& srcPath) {
    TUnbufferedFileInput src(srcPath);
    TUnbufferedFileOutput dst(TFile(dstPath, ForAppend | WrOnly | Seq));

    TransferData(&src, &dst);
}

void NFs::Copy(const std::string& existingPath, const std::string& newPath) {
    TUnbufferedFileInput src(existingPath);
    TUnbufferedFileOutput dst(TFile(newPath, CreateAlways | WrOnly | Seq));

    TransferData(&src, &dst);
}

bool NFs::Exists(const std::string& path) {
#if defined(_win_)
    return NFsPrivate::WinExists(path);
#elif defined(_unix_)
    return access(path.data(), F_OK) == 0;
#endif
}

std::string NFs::CurrentWorkingDirectory() {
#if defined(_win_)
    return NFsPrivate::WinCurrentWorkingDirectory();
#elif defined(_unix_)
    TTempBuf result;
    char* r = getcwd(result.Data(), result.Size());
    if (r == nullptr) {
        throw TIoSystemError() << "failed to getcwd";
    }
    return result.Data();
#endif
}

void NFs::SetCurrentWorkingDirectory(const std::string& path) {
#ifdef _win_
    bool ok = NFsPrivate::WinSetCurrentWorkingDirectory(path);
#else
    bool ok = !chdir(path.data());
#endif
    if (!ok) {
        ythrow TSystemError() << "failed to change directory to " << NUtils::Quote(path);
    }
}
