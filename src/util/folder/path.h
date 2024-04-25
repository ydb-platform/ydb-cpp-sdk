#pragma once

#include "fwd.h"
#include "pathsplit.h"

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/string/cast.h>
#include <src/util/system/fstat.h>
#include <ydb-cpp-sdk/util/system/platform.h>
#include <ydb-cpp-sdk/util/system/sysstat.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
<<<<<<< HEAD
=======
#include <src/util/generic/ptr.h>
#include <src/util/string/cast.h>
#include <src/util/system/fstat.h>
#include <src/util/system/platform.h>
#include <src/util/system/sysstat.h>
#include <src/util/system/yassert.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

#include <utility>

/**
 * Class behaviour is platform-dependent.
 * It uses platform-dependent separators for path-reconstructing operations.
 */
class TFsPath {
private:
    struct TSplit;

public:
    TFsPath();
    TFsPath(const std::string& path);
    TFsPath(const std::string_view path);
    TFsPath(const char* path);

    TFsPath(const TFsPath& that);
    TFsPath(TFsPath&& that);

    TFsPath& operator=(const TFsPath& that);
    TFsPath& operator=(TFsPath&& that);

    ~TFsPath() = default;

    void CheckDefined() const;

    inline bool IsDefined() const {
        return Path_.length() > 0;
    }

    inline explicit operator bool() const {
        return IsDefined();
    }

    inline const char* c_str() const {
        return Path_.c_str();
    }

    inline operator const std::string&() const {
        return Path_;
    }

    inline bool operator==(const TFsPath& that) const {
        return Path_ == that.Path_;
    }

    TFsPath& operator/=(const TFsPath& that);

    friend TFsPath operator/(const TFsPath& s, const TFsPath& p) {
        TFsPath ret(s);
        return ret /= p;
    }

    const TPathSplit& PathSplit() const;

    TFsPath& Fix();

    inline const std::string& GetPath() const {
        return Path_;
    }

    /// last component of path, or "/" if root
    std::string GetName() const;

    /**
     * "a.b.tmp" -> "tmp"
     * "a.tmp"   -> "tmp"
     * ".tmp"    -> ""
     */
    std::string GetExtension() const;

    bool IsAbsolute() const;
    bool IsRelative() const;

    /**
     * TFsPath("/a/b").IsSubpathOf("/a")        -> true
     *
     * TFsPath("/a").IsSubpathOf("/a")          -> false
     *
     * TFsPath("/a").IsSubpathOf("/other/path") -> false
     * @param that - presumable parent path of this
     * @return True if this is a subpath of that and false otherwise.
     */
    bool IsSubpathOf(const TFsPath& that) const;

    /**
     * TFsPath("/a/b").IsNonStrictSubpathOf("/a")        -> true
     *
     * TFsPath("/a").IsNonStrictSubpathOf("/a")          -> true
     *
     * TFsPath("/a").IsNonStrictSubpathOf("/other/path") -> false
     * @param that - presumable parent path of this
     * @return True if this is a subpath of that or they are equivalent and false otherwise.
     */
    bool IsNonStrictSubpathOf(const TFsPath& that) const;

    bool IsContainerOf(const TFsPath& that) const {
        return that.IsSubpathOf(*this);
    }

    TFsPath RelativeTo(const TFsPath& root) const; // must be subpath of root

    /**
     * @returns relative path or empty path if root equals to this.
     */
    TFsPath RelativePath(const TFsPath& root) const; //..; for relative paths 1st component must be the same

    /**
     * Never fails. Returns this if already a root.
     */
    TFsPath Parent() const;

    std::string Basename() const {
        return GetName();
    }
    std::string Dirname() const {
        return Parent();
    }

    TFsPath Child(const std::string& name) const;

    /**
     * @brief create this directory
     *
     * @param mode specifies permissions to use as described in mkdir(2), makes sense only on Unix-like systems.
     *
     * Nothing to do if dir exists.
     */
    void MkDir(const int mode = MODE0777) const;

    /**
     * @brief create this directory and all parent directories as needed
     *
     * @param mode specifies permissions to use as described in mkdir(2), makes sense only on Unix-like systems.
     */
    void MkDirs(const int mode = MODE0777) const;

    // XXX: rewrite to return iterator
    void List(std::vector<TFsPath>& children) const;
    void ListNames(std::vector<std::string>& children) const;

    // Check, if path contains at least one component with a specific name.
    bool Contains(const std::string& component) const;

    // fails to delete non-empty directory
    void DeleteIfExists() const;
    // delete recursively. Does nothing if not exists
    void ForceDelete() const;

    // XXX: ino

    inline bool Stat(TFileStat& stat) const {
        stat = TFileStat(Path_.data());

        return stat.Mode;
    }

    bool Exists() const;
    /// false if not exists
    bool IsDirectory() const;
    /// false if not exists
    bool IsFile() const;
    /// false if not exists
    bool IsSymlink() const;
    /// throw TIoException if not exists
    void CheckExists() const;

    void RenameTo(const std::string& newPath) const;
    void RenameTo(const char* newPath) const;
    void RenameTo(const TFsPath& newFile) const;
    void ForceRenameTo(const std::string& newPath) const;

    void CopyTo(const std::string& newPath, bool force) const;

    void Touch() const;

    TFsPath RealPath() const;
    TFsPath RealLocation() const;
    TFsPath ReadLink() const;

    /// always absolute
    static TFsPath Cwd();

    inline void Swap(TFsPath& p) noexcept {
        DoSwap(Path_, p.Path_);
        Split_.Swap(p.Split_);
    }

private:
    void InitSplit() const;
    TSplit& GetSplit() const;

private:
    std::string Path_;
    /// cache
    mutable TSimpleIntrusivePtr<TSplit> Split_;
};

namespace NPrivate {
    inline void AppendToFsPath(TFsPath&) {
    }

    template <class T, class... Ts>
    void AppendToFsPath(TFsPath& fsPath, const T& arg, Ts&&... args) {
        fsPath /= TFsPath(arg);
        AppendToFsPath(fsPath, std::forward<Ts>(args)...);
    }
}

template <class... Ts>
std::string JoinFsPaths(Ts&&... args) {
    TFsPath fsPath;
    ::NPrivate::AppendToFsPath(fsPath, std::forward<Ts>(args)...);
    return fsPath.GetPath();
}
