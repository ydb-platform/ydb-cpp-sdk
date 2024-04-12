#include "tempfile.h"

TTempFileHandle::TTempFileHandle()
    : TTempFile(MakeTempName())
    , TFile(CreateFile())
{
}

TTempFileHandle::TTempFileHandle(const std::string& fname)
    : TTempFile(fname)
    , TFile(CreateFile())
{
}

TTempFileHandle TTempFileHandle::InCurrentDir(const std::string& filePrefix, const std::string& extension) {
    return TTempFileHandle(MakeTempName(".", filePrefix.c_str(), extension.c_str()));
}

TTempFileHandle TTempFileHandle::InDir(const TFsPath& dirPath, const std::string& filePrefix, const std::string& extension) {
    return TTempFileHandle(MakeTempName(dirPath.c_str(), filePrefix.c_str(), extension.c_str()));
}

TFile TTempFileHandle::CreateFile() const {
    return TFile(Name(), CreateAlways | RdWr);
}
