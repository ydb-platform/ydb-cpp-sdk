<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/platform.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/system/backtrace.h>
=======
#include <src/util/system/platform.h>
#include <src/util/system/yassert.h>
#include <src/util/stream/output.h>
#include <src/util/system/backtrace.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <string.h>

#ifdef _unix_
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "segv_handler.h"

#ifndef _win_
static void SegvHandler(int sig) {
    Y_UNUSED(sig);
    const char msg[] = "Got SEGV\n";
    Y_UNUSED(write(STDERR_FILENO, msg, sizeof(msg)));
    //PrintBackTrace();
    sig_t r = signal(SIGSEGV, SIG_DFL);
    if (r == SIG_ERR) {
        abort();
    }
    // returning back and failing
}
#endif // !_win_

void InstallSegvHandler() {
#ifndef _win_
    sig_t r = signal(SIGSEGV, &SegvHandler);
    Y_ABORT_UNLESS(r != SIG_ERR, "signal failed: %s", strerror(errno));
#endif // !_win_
}
