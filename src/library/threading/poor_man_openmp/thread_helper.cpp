#include "thread_helper.h"

#include <src/util/generic/singleton.h>

TMtpQueueHelper& TMtpQueueHelper::Instance() {
    return *Singleton<TMtpQueueHelper>();
}
