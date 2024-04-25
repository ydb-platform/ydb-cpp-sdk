#include "user.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main

#ifdef _win_
    #include "winint.h"
#else
    #include <cerrno>
    #include <pwd.h>
    #include <unistd.h>
#endif

std::string GetUsername() {
    for (const auto& var : {"LOGNAME", "USER", "LNAME", "USERNAME"}) {
        std::string val = std::getenv(var) ? std::getenv(var) : "";
        if (!val.empty()) {
            return val;
        }
    }

    TTempBuf nameBuf;
    for (;;) {
#if defined(_win_)
        DWORD len = (DWORD)Min(nameBuf.Size(), size_t(32767));
        if (!GetUserNameA(nameBuf.Data(), &len)) {
            DWORD err = GetLastError();
            if ((err == ERROR_INSUFFICIENT_BUFFER) && (nameBuf.Size() <= 32767))
                nameBuf = TTempBuf((size_t)len);
            else
                ythrow TSystemError(err) << " GetUserName failed";
        } else {
            return std::string(nameBuf.Data(), (size_t)(len - 1));
        }
#elif defined(_bionic_)
        const passwd* pwd = getpwuid(geteuid());

        if (pwd) {
            return std::string(pwd->pw_name);
        }

        ythrow TSystemError() << std::string_view(" getpwuid failed");
#else
        passwd pwd;
        passwd* tmpPwd;
        int err = getpwuid_r(geteuid(), &pwd, nameBuf.Data(), nameBuf.Size(), &tmpPwd);
        if (err == 0 && tmpPwd) {
            return std::string(pwd.pw_name);
        } else if (err == ERANGE) {
            nameBuf = TTempBuf(nameBuf.Size() * 2);
        } else {
            ythrow TSystemError(err) << " getpwuid_r failed";
        }
#endif
    }
}
