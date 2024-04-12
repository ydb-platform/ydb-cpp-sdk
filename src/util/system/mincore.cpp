#include "align.h"
#include "info.h"
#include "mincore.h"

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

#if defined(_unix_)
    #include <sys/mman.h>
    #if defined(_android_)
        #include <sys/syscall.h>
    #endif
#endif

void InCoreMemory(const void* addr, size_t len, unsigned char* vec, size_t vecLen) {
#if defined(_linux_)
    const size_t pageSize = NSystemInfo::GetPageSize();
    void* maddr = const_cast<void*>(AlignDown(addr, pageSize));
    len = AlignUp(len, pageSize);
    if (vecLen * pageSize < len) {
        ythrow yexception() << "vector argument for mincore is too small: " << vecLen * pageSize << " < " << len;
    }
    if (::mincore(maddr, len, vec)) {
        ythrow yexception() << LastSystemErrorText();
    }
#else
    // pessimistic assumption: nothing is in core
    Y_UNUSED(addr);
    Y_UNUSED(len);
    ::memset(vec, 0, vecLen);
#endif
}
