#include "tempfile.h"

#include <src/util/folder/dirut.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <cstring>

#ifdef _win32_
    #include "winint.h"
    #include <io.h>
#else
    #include <unistd.h>
#endif

extern "C" int mkstemps(char* path, int slen);

std::string MakeTempName(const char* wrkDir, const char* prefix, const char* extension) {
#ifndef _win32_
    std::string filePath;

    if (wrkDir && *wrkDir) {
        filePath += wrkDir;
    } else {
        filePath += GetSystemTempDir();
    }

    if (filePath.back() != '/') {
        filePath += '/';
    }

    if (prefix) {
        filePath += prefix;
    }

    filePath += "XXXXXX"; // mkstemps requirement

    size_t extensionPartLength = 0;
    if (extension && *extension) {
        if (extension[0] != '.') {
            filePath += '.';
            extensionPartLength += 1;
        }
        filePath += extension;
        extensionPartLength += strlen(extension);
    }

    int fd = mkstemps(const_cast<char*>(filePath.data()), extensionPartLength);
    if (fd >= 0) {
        close(fd);
        return filePath;
    }
#else
    char tmpDir[MAX_PATH + 1]; // +1 -- for terminating null character
    char filePath[MAX_PATH];
    const char* pDir = 0;

    if (wrkDir && *wrkDir) {
        pDir = wrkDir;
    } else if (GetTempPath(MAX_PATH + 1, tmpDir)) {
        pDir = tmpDir;
    }

    // it always takes up to 3 characters, no more
    if (GetTempFileName(pDir, (prefix) ? (prefix) : "yan", 0, filePath)) {
        return filePath;
    }
#endif

    ythrow TSystemError() << "can not create temp name(" << wrkDir << ", " << prefix << ", " << extension << ")";
}
