#pragma once

#include <string>
#include <ydb-cpp-sdk/util/datetime/base.h>

namespace NCoro {

    // @brief check that address  `host`:`port` is connectable
    bool TryConnect(const std::string& host, ui16 port, TDuration timeout = TDuration::Seconds(1));

    // @brief waits until address `host`:`port` became connectable, but not more than timeout
    // @return true on success, false if timeout exceeded
    bool WaitUntilConnectable(const std::string& host, ui16 port, TDuration timeout);

}
