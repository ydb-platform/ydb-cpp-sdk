#pragma once

#include <userver/ydb/builder.hpp>
#include <userver/ydb/table.hpp>

#include <ydb-cpp-sdk/client/types/credentials/credentials.h>
#include <ydb-cpp-sdk/client/value/value.h>

#include <memory>
#include <string>
#include <vector>

namespace userver_slo {

void InitTableClient(
    const std::string& endpoint,
    const std::string& database,
    const std::shared_ptr<NYdb::ICredentialsProviderFactory>& credentials_provider_factory,
    const std::string& oauth_token,
    bool prefer_local_dc
);

userver::ydb::TableClient& GetTableClient();

void ShutdownTableClient();

userver::ydb::PreparedArgsBuilder PackValuesToPreparedArgs(
    const std::vector<NYdb::TValue>& items,
    const std::string& name = "$items"
);

}  // namespace userver_slo
