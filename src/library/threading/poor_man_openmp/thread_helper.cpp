#include "thread_helper.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/singleton.h>
=======
#include <src/util/generic/singleton.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

TMtpQueueHelper& TMtpQueueHelper::Instance() {
    return *Singleton<TMtpQueueHelper>();
}
