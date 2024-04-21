from util.generic.string cimport std::string, std::string_view
from util.generic.vector cimport TVector


# NOTE (danila-eremin) Currently not possible to use `const` and `except +` at the same time, so some function not marked const
cdef extern from "src/util/folder/path.h" nogil:
    cdef cppclass TFsPath:
        TFsPath() except +
        TFsPath(const std::string&) except +
        TFsPath(const std::string_view) except +
        TFsPath(const char*) except +

        void CheckDefined() except +

        bint IsDefined() const
        bint operator bool() const

        const char* c_str() const

        bint operator==(const TFsPath&) const
        bint operator!=(const TFsPath&) const

        # NOTE (danila-eremin) operator `/=` Not supported
        # TFsPath& operator/=(const TFsPath&) const

        TFsPath operator/(const TFsPath&, const TFsPath&) except +

        # NOTE (danila-eremin) TPathSplit needed
        # const TPathSplit& PathSplit() const

        TFsPath& Fix() except +

        const std::string& GetPath() const
        std::string GetName() const

        std::string GetExtension() const

        bint IsAbsolute() const
        bint IsRelative() const

        bint IsSubpathOf(const TFsPath&) const
        bint IsNonStrictSubpathOf(const TFsPath&) const
        bint IsContainerOf(const TFsPath&) const

        TFsPath RelativeTo(const TFsPath&) except +
        TFsPath RelativePath(const TFsPath&) except +

        TFsPath Parent() const

        std::string Basename() const
        std::string Dirname() const

        TFsPath Child(const std::string&) except +

        void MkDir() except +
        void MkDir(const int) except +
        void MkDirs() except +
        void MkDirs(const int) except +

        void List(TVector[TFsPath]&) except +
        void ListNames(TVector[std::string]&) except +

        bint Contains(const std::string&) const

        void DeleteIfExists() except +
        void ForceDelete() except +

        # NOTE (danila-eremin) TFileStat needed
        # bint Stat(TFileStat&) const

        bint Exists() const
        bint IsDirectory() const
        bint IsFile() const
        bint IsSymlink() const
        void CheckExists() except +

        void RenameTo(const std::string&) except +
        void RenameTo(const char*) except +
        void RenameTo(const TFsPath&) except +
        void ForceRenameTo(const std::string&) except +

        void CopyTo(const std::string&, bint) except +

        void Touch() except +

        TFsPath RealPath() except +
        TFsPath RealLocation() except +
        TFsPath ReadLink() except +

        @staticmethod
        TFsPath Cwd() except +

        void Swap(TFsPath&)
