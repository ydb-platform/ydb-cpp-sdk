#include "thread_helper.h"

#include <ydb-cpp-sdk/util/generic/singleton.h>

TMtpQueueHelper& TMtpQueueHelper::Instance() {
    return *Singleton<TMtpQueueHelper>();
}
