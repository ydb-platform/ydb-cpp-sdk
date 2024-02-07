#pragma once

#include <client/impl/ydb_internal/common/type_switcher.h>
#include <util/datetime/base.h>

namespace NYdb {

struct TJwtParams {
    std::stringType PrivKey;
    std::stringType PubKey;
    std::stringType AccountId;
    std::stringType KeyId;
};

TJwtParams ParseJwtParams(const std::stringType& jsonParamsStr);
std::stringType MakeSignedJwt(
    const TJwtParams& params,
    const TDuration& lifetime = TDuration::Hours(1)
);

} // namespace NYdb
