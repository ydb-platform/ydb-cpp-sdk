#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/system/fhandle.h>
=======
#include <src/util/generic/fwd.h>
#include <src/util/system/fhandle.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TFile;
class TFsPath;
struct stat;

struct TFileStat {
    ui32 Mode = 0; /* protection */
    ui32 Uid = 0;  /* user ID of owner */
    ui32 Gid = 0;  /* group ID of owner */

    ui64 NLinks = 0;         /* number of hard links */
    ui64 Size = 0;           /* total size, in bytes */
    ui64 INode = 0;          /* inode number */
    ui64 AllocationSize = 0; /* number of bytes allocated on the disk */

    time_t ATime = 0; /* time of last access */
    time_t MTime = 0; /* time of last modification */
    time_t CTime = 0; /* time of last status change */

public:
    TFileStat();

    bool IsNull() const noexcept;

    bool IsFile() const noexcept;
    bool IsDir() const noexcept;
    bool IsSymlink() const noexcept;

    explicit TFileStat(const struct stat& fs);
    explicit TFileStat(const TFile& f);
    explicit TFileStat(FHANDLE f);
    TFileStat(const TFsPath& fileName, bool nofollow = false);
    TFileStat(const std::string& fileName, bool nofollow = false);
    TFileStat(const char* fileName, bool nofollow = false);

    friend bool operator==(const TFileStat& l, const TFileStat& r) noexcept;
    friend bool operator!=(const TFileStat& l, const TFileStat& r) noexcept;

private:
    void MakeFromFileName(const char* fileName, bool nofollow);
};

i64 GetFileLength(FHANDLE fd);
i64 GetFileLength(const char* name);
i64 GetFileLength(const std::string& name);
