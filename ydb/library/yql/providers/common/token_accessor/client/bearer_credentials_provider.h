#pragma once

#include <client/ydb_types/credentials/credentials.h>

namespace NYql {

std::shared_ptr<NYdb::ICredentialsProviderFactory> WrapCredentialsProviderFactoryWithBearer(
    std::shared_ptr<NYdb::ICredentialsProviderFactory> delegatee
);

}
