#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/types.h>
=======
#include <src/util/system/types.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/types.h>
>>>>>>> origin/main

namespace NYdb {

constexpr ui64 TCP_KEEPALIVE_IDLE = 30; // The time the connection needs to remain idle
                                        // before TCP starts sending keepalive probes, seconds
constexpr ui64 TCP_KEEPALIVE_COUNT = 5; // The maximum number of keepalive probes TCP should send before
                                        // dropping the connection
constexpr ui64 TCP_KEEPALIVE_INTERVAL = 10; // The time between individual keepalive probes, seconds

} // namespace NYdb
