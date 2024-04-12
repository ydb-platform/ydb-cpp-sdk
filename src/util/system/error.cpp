#include <ydb-cpp-sdk/util/system/error.h>

#include <src/util/generic/strfcpy.h>

#include <cerrno>
#include <cstring>

#if defined(_win_)
<<<<<<< HEAD
    #include <ydb-cpp-sdk/util/string/strip.h>
    #include <ydb-cpp-sdk/util/network/socket.h>
    #include <ydb-cpp-sdk/util/generic/singleton.h>
=======
    #include <src/util/string/strip.h>
    #include <src/util/network/socket.h>
    #include <src/util/generic/singleton.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
    #include "winint.h"
#endif

void ClearLastSystemError() {
#if defined(_win_)
    SetLastError(0);
#else
    errno = 0;
#endif
}

int LastSystemError() {
#if defined(_win_)
    int ret = GetLastError();

    if (ret)
        return ret;

    ret = WSAGetLastError();

    if (ret)
        return ret;
    // when descriptors number are over maximum, errno set in this variable
    ret = *(_errno());
    return ret;

#else
    return errno;
#endif
}

#if defined(_win_)
namespace {
    struct TErrString {
        inline TErrString() noexcept {
            data[0] = 0;
        }

        char data[1024];
    };
}
#endif

const char* LastSystemErrorText(int code) {
#if defined(_win_)
    TErrString& text(*Singleton<TErrString>());
    LastSystemErrorText(text.data, sizeof(text.data), code);

    return text.data;
#else
    return strerror(code);
#endif
}

#ifdef _win_
static char* Strip(char* s) {
    size_t len = strlen(s);
    const char* ptr = s;
    Strip(ptr, len);
    if (ptr != s)
        memmove(s, ptr, len);
    s[len] = 0;
    return s;
}
#endif // _win_

void LastSystemErrorText(char* str, size_t size, int code) {
#if defined(_win_)
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, code, 0, str, DWORD(size), 0);
    Strip(str);
#elif defined(_sun_)
    strfcpy(str, strerror(code), size);
#elif defined(_freebsd_) || defined(_darwin_) || defined(_musl_) || defined(_bionic_)
    strerror_r(code, str, size);
#elif defined(_linux_) | defined(_cygwin_)
    char* msg = strerror_r(code, str, size);
    strncpy(str, msg, size);
#else
    #error port me gently!
#endif
}
