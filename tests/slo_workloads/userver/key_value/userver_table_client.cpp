#include "userver_table_client.h"

#include <userver/dynamic_config/test_helpers.hpp>
#include <userver/ydb/settings.hpp>

#include <ydb/impl/config.hpp>
#include <ydb/impl/driver.hpp>

namespace userver_slo {
namespace {

struct Clients final {
    std::shared_ptr<userver::ydb::impl::Driver> driver;
    std::shared_ptr<userver::ydb::TableClient> table_client;
};

Clients& GetClients() {
    static Clients clients;
    return clients;
}

}  // namespace

void InitTableClient(
    const std::string& endpoint,
    const std::string& database,
    const std::shared_ptr<NYdb::ICredentialsProviderFactory>& credentials_provider_factory,
    const std::string& oauth_token,
    const bool prefer_local_dc
) {
    auto& clients = GetClients();
    if (clients.table_client) {
        return;
    }

    userver::ydb::impl::DriverSettings driver_settings;
    driver_settings.endpoint = endpoint;
    driver_settings.database = database;
    driver_settings.prefer_local_dc = prefer_local_dc;
    driver_settings.credentials_provider_factory = credentials_provider_factory;
    if (!oauth_token.empty()) {
        driver_settings.oauth_token = oauth_token;
    }

    const userver::ydb::impl::TableSettings table_settings;
    const userver::ydb::OperationSettings operation_settings{
        3,
        std::chrono::minutes{3},
        std::chrono::minutes{3},
        std::chrono::minutes{3},
        userver::ydb::TransactionMode::kSerializableRW,
        std::chrono::minutes{3},
    };

    clients.driver = std::make_shared<userver::ydb::impl::Driver>("slo", std::move(driver_settings));
    clients.table_client = std::make_shared<userver::ydb::TableClient>(
        table_settings,
        operation_settings,
        userver::dynamic_config::GetDefaultSource(),
        clients.driver
    );
}

userver::ydb::TableClient& GetTableClient() {
    auto& clients = GetClients();
    if (!clients.table_client) {
        throw std::runtime_error("userver TableClient is not initialized");
    }
    return *clients.table_client;
}

void ShutdownTableClient() {
    auto& clients = GetClients();
    clients.table_client.reset();
    clients.driver.reset();
}

userver::ydb::PreparedArgsBuilder PackValuesToPreparedArgs(
    const std::vector<NYdb::TValue>& items,
    const std::string& name
) {
    NYdb::TValueBuilder items_as_list;
    items_as_list.BeginList();
    for (const auto& item : items) {
        items_as_list.AddListItem(item);
    }
    items_as_list.EndList();

    NYdb::TParamsBuilder params_builder;
    params_builder.AddParam(name, items_as_list.Build());
    return userver::ydb::PreparedArgsBuilder(std::move(params_builder));
}

}  // namespace userver_slo
