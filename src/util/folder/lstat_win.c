<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>

#ifdef _win_
    #include <ydb-cpp-sdk/util/system/winint.h>
=======
#include <src/util/system/defaults.h>

#ifdef _win_
    #include <src/util/system/winint.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
    #include "lstat_win.h"

int lstat(const char* fileName, stat_struct* fileStat) {
    int len = strlen(fileName);
    int convRes = MultiByteToWideChar(CP_UTF8, 0, fileName, len, 0, 0);
    if (convRes == 0) {
        return -1;
    }
    WCHAR* buf = malloc(sizeof(WCHAR) * (convRes + 1));
    MultiByteToWideChar(CP_UTF8, 0, fileName, len, buf, convRes);
    buf[convRes] = 0;

    HANDLE findHandle;
    WIN32_FIND_DATAW findBuf;
    int result;
    result = _wstat64(buf, fileStat);
    if (result == 0) {
        SetLastError(0);
        findHandle = FindFirstFileW(buf, &findBuf);
        if (findBuf.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT &&
            (findBuf.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT || findBuf.dwReserved0 == IO_REPARSE_TAG_SYMLINK))
        {
            fileStat->st_mode = fileStat->st_mode & ~_S_IFMT | _S_IFLNK;
        }
        FindClose(findHandle);
    }
    free(buf);
    return result;
}

#endif //_win_
