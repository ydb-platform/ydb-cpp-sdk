#pragma once

#include <string>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NCoro {

    // @brief check that address  `host`:`port` is connectable
    bool TryConnect(const std::string& host, ui16 port, TDuration timeout = TDuration::Seconds(1));

    // @brief waits until address `host`:`port` became connectable, but not more than timeout
    // @return true on success, false if timeout exceeded
    bool WaitUntilConnectable(const std::string& host, ui16 port, TDuration timeout);

}
