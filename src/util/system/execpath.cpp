#include <ydb-cpp-sdk/util/system/platform.h>

#if defined(_solaris_)
    #include <stdlib.h>
#elif defined(_darwin_)
    #include <mach-o/dyld.h>
    #include <src/util/generic/function.h>
#elif defined(_win_)
    #include "winint.h"
    #include <io.h>
#elif defined(_linux_)
#elif defined(_freebsd_)
    #include <string.h>
    #include <sys/types.h> // for u_int not defined in sysctl.h
    #include <sys/sysctl.h>
    #include <unistd.h>
#endif

#include <src/util/generic/singleton.h>

#include "execpath.h"
#include "fs.h"

#if defined(_freebsd_)
static inline bool GoodPath(const std::string& path) {
    return path.find('/') != std::string::npos;
}

static inline int FreeBSDSysCtl(int* mib, size_t mibSize, TTempBuf& res) {
    for (size_t i = 0; i < 2; ++i) {
        size_t cb = res.Size();
        if (sysctl(mib, mibSize, res.Data(), &cb, nullptr, 0) == 0) {
            res.Proceed(cb);
            return 0;
        } else if (errno == ENOMEM) {
            res = TTempBuf(cb);
        } else {
            return errno;
        }
    }
    return errno;
}

static inline std::string FreeBSDGetExecPath() {
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    TTempBuf buf;
    int r = FreeBSDSysCtl(mib, Y_ARRAY_SIZE(mib), buf);
    if (r == 0) {
        return std::string(buf.Data(), buf.Filled() - 1);
    } else if (r == ENOTSUP) { // older FreeBSD version
        /*
         * BSD analogue for /proc/self is /proc/curproc.
         * See:
         * https://www.freebsd.org/cgi/man.cgi?query=procfs&sektion=5&format=html
         */
        std::string path("/proc/curproc/file");
        return NFs::ReadLink(path);
    } else {
        return std::string();
    }
}

static inline std::string FreeBSDGetArgv0() {
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ARGS, getpid()};
    TTempBuf buf;
    int r = FreeBSDSysCtl(mib, Y_ARRAY_SIZE(mib), buf);
    if (r == 0) {
        return std::string(buf.Data());
    } else if (r == ENOTSUP) {
        return std::string();
    } else {
        ythrow yexception() << "FreeBSDGetArgv0() failed: " << LastSystemErrorText();
    }
}

static inline bool FreeBSDGuessExecPath(const std::string& guessPath, std::string& execPath) {
    if (NFs::Exists(guessPath)) {
        // now it should work for real
        execPath = FreeBSDGetExecPath();
        if (RealPath(execPath) == RealPath(guessPath)) {
            return true;
        }
    }
    return false;
}

static inline bool FreeBSDGuessExecBasePath(const std::string& guessBasePath, std::string& execPath) {
    return FreeBSDGuessExecPath(std::string(guessBasePath) + "/" + getprogname(), execPath);
}

#endif

static std::string GetExecPathImpl() {
#if defined(_solaris_)
    return execname();
#elif defined(_darwin_)
    TTempBuf execNameBuf;
    for (size_t i = 0; i < 2; ++i) {
        std::remove_pointer_t<TFunctionArg<decltype(_NSGetExecutablePath), 1>> bufsize = execNameBuf.Size();
        int r = _NSGetExecutablePath(execNameBuf.Data(), &bufsize);
        if (r == 0) {
            return execNameBuf.Data();
        } else if (r == -1) {
            execNameBuf = TTempBuf(bufsize);
        }
    }
    ythrow yexception() << "GetExecPathImpl() failed";
#elif defined(_win_)
    TTempBuf execNameBuf;
    for (;;) {
        DWORD r = GetModuleFileName(nullptr, execNameBuf.Data(), execNameBuf.Size());
        if (r == execNameBuf.Size()) {
            execNameBuf = TTempBuf(execNameBuf.Size() * 2);
        } else if (r == 0) {
            ythrow yexception() << "GetExecPathImpl() failed: " << LastSystemErrorText();
        } else {
            return execNameBuf.Data();
        }
    }
#elif defined(_linux_) || defined(_cygwin_)
    std::string path("/proc/self/exe");
    return NFs::ReadLink(path);
// TODO(yoda): check if the filename ends with " (deleted)"
#elif defined(_freebsd_)
    std::string execPath = FreeBSDGetExecPath();
    if (GoodPath(execPath)) {
        return execPath;
    }
    if (FreeBSDGuessExecPath(FreeBSDGetArgv0(), execPath)) {
        return execPath;
    }
    if (FreeBSDGuessExecPath(getenv("_"), execPath)) {
        return execPath;
    }
    if (FreeBSDGuessExecBasePath(getenv("PWD"), execPath)) {
        return execPath;
    }
    if (FreeBSDGuessExecBasePath(NFs::CurrentWorkingDirectory(), execPath)) {
        return execPath;
    }

    ythrow yexception() << "can not resolve exec path";
#else
    #error dont know how to implement GetExecPath on this platform
#endif
}

static bool GetPersistentExecPathImpl(std::string& to) {
#if defined(_solaris_)
    to = std::string("/proc/self/object/a.out");
    return true;
#elif defined(_linux_) || defined(_cygwin_)
    to = std::string("/proc/self/exe");
    return true;
#elif defined(_freebsd_)
    to = std::string("/proc/curproc/file");
    return true;
#else // defined(_win_) || defined(_darwin_)  or unknown
    Y_UNUSED(to);
    return false;
#endif
}

namespace {
    struct TExecPathsHolder {
        inline TExecPathsHolder() {
            ExecPath = GetExecPathImpl();

            if (!GetPersistentExecPathImpl(PersistentExecPath)) {
                PersistentExecPath = ExecPath;
            }
        }

        static inline auto Instance() {
            return SingletonWithPriority<TExecPathsHolder, 1>();
        }

        std::string ExecPath;
        std::string PersistentExecPath;
    };
}

const std::string& GetExecPath() {
    return TExecPathsHolder::Instance()->ExecPath;
}

const std::string& GetPersistentExecPath() {
    return TExecPathsHolder::Instance()->PersistentExecPath;
}
