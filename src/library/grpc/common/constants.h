#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/grpc/common/constants.h
#include <ydb-cpp-sdk/util/system/types.h>
========
#include <src/util/system/types.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/grpc/common/constants.h

namespace NYdbGrpc {

constexpr ui64 DEFAULT_GRPC_MESSAGE_SIZE_LIMIT = 64000000;

}
