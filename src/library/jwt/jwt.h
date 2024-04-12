#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/jwt/jwt.h
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/datetime/base.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/jwt/jwt.h

namespace NYdb {

struct TJwtParams {
    std::string PrivKey;
    std::string PubKey;
    std::string AccountId;
    std::string KeyId;
};

TJwtParams ParseJwtParams(const std::string& jsonParamsStr);
std::string MakeSignedJwt(
    const TJwtParams& params,
    const TDuration& lifetime = TDuration::Hours(1)
);

} // namespace NYdb
