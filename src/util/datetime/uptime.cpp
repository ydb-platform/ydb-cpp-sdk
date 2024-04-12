#include "uptime.h"

#if defined(_win_)
<<<<<<< HEAD
    #include <ydb-cpp-sdk/util/system/winint.h>
#elif defined(_linux_)
    #include <src/util/stream/file.h>
    #include <ydb-cpp-sdk/util/string/cast.h>
=======
    #include <src/util/system/winint.h>
#elif defined(_linux_)
    #include <src/util/stream/file.h>
    #include <src/util/string/cast.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
#elif defined(_darwin_)
    #include <sys/sysctl.h>
#endif

#if defined(_darwin_)
namespace {
    TInstant GetBootTime() {
        struct timeval timeSinceBoot;
        size_t len = sizeof(timeSinceBoot);
        int request[2] = {CTL_KERN, KERN_BOOTTIME};
        if (sysctl(request, 2, &timeSinceBoot, &len, nullptr, 0) < 0) {
            ythrow yexception() << "cannot get kern.boottime from sysctl";
        }
        return TInstant::MicroSeconds(timeSinceBoot.tv_sec * 1'000'000 + timeSinceBoot.tv_usec);
    }

    TDuration GetDarwinUptime() {
        TInstant beforeNow;
        TInstant afterNow;
        TInstant now;

        // avoid race when NTP changes machine time between getting Now() and uptime
        afterNow = GetBootTime();
        do {
            beforeNow = afterNow;
            now = Now();
            afterNow = GetBootTime();
        } while (afterNow != beforeNow);

        return now - beforeNow;
    }
}
#endif // _darwin_

TDuration Uptime() {
#if defined(_win_)
    return TDuration::MilliSeconds(GetTickCount64());
#elif defined(_linux_)
    TUnbufferedFileInput in("/proc/uptime");
    std::string uptimeStr = in.ReadLine();
    double up, idle;
    if (sscanf(uptimeStr.data(), "%lf %lf", &up, &idle) < 2) {
        ythrow yexception() << "cannot read values from /proc/uptime";
    }
    return TDuration::MilliSeconds(up * 1000.0);
#elif defined(_darwin_)
    return GetDarwinUptime();
#endif
}
