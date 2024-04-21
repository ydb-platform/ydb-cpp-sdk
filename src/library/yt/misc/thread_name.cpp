#include "thread_name.h"

#include <src/library/yt/misc/tls.h>

#include <string>
#include <src/util/system/thread.h>

#include <algorithm>
#include <cstring>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

std::string_view TThreadName::ToStringBuf() const
{
    // Buffer is zero terminated.
    return std::string_view(Buffer.data(), Length);
}

////////////////////////////////////////////////////////////////////////////////

TThreadName::TThreadName(const std::string& name)
{
    Length = std::min<int>(TThreadName::BufferCapacity - 1, name.length());
    ::memcpy(Buffer.data(), name.data(), Length);
}

////////////////////////////////////////////////////////////////////////////////

// This function uses cached TThread::CurrentThreadName() result
TThreadName GetCurrentThreadName()
{
    static YT_THREAD_LOCAL(TThreadName) ThreadName;
    auto& threadName = GetTlsRef(ThreadName);

    if (threadName.Length == 0) {
        auto name = TThread::CurrentThreadName();
        if (!name.empty()) {
            auto length = std::min<int>(TThreadName::BufferCapacity - 1, name.length());
            threadName.Length = length;
            ::memcpy(threadName.Buffer.data(), name.data(), length);
        }
    }
    return threadName;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
