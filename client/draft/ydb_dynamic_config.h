#pragma once

#include <ydb/public/api/grpc/draft/ydb_dynamic_config_v1.grpc.pb.h>
#include <client/ydb_types/ydb.h>
#include <client/ydb_types/status/status.h>
#include <client/ydb_common_client/settings.h>
#include <client/ydb_types/request_settings.h>
#include <client/ydb_driver/driver.h>

#include <util/generic/set.h>
#include <util/system/types.h>

#include <memory>

namespace NYdb::NDynamicConfig {

struct TGetConfigResult : public TStatus {
    TGetConfigResult(
        TStatus&& status,
        std::string&& clusterName,
        ui64 version,
        std::string&& config,
        std::map<ui64, std::string>&& volatileConfigs)
        : TStatus(std::move(status))
        , ClusterName_(std::move(clusterName))
        , Version_(version)
        , Config_(std::move(config))
        , VolatileConfigs_(std::move(volatileConfigs))
    {}

    const std::string& GetClusterName() const {
        return ClusterName_;
    }

    ui64 GetVersion() const {
        return Version_;
    }

    const std::string& GetConfig() const {
        return Config_;
    }

    const std::map<ui64, std::string>& GetVolatileConfigs() const {
        return VolatileConfigs_;
    }

private:
    std::string ClusterName_;
    ui64 Version_;
    std::string Config_;
    std::map<ui64, std::string> VolatileConfigs_;
};

using TAsyncGetConfigResult = NThreading::TFuture<TGetConfigResult>;

struct TGetMetadataResult : public TStatus {
    TGetMetadataResult(
        TStatus&& status,
        std::string&& metadata,
        std::map<ui64, std::string>&& volatileConfigs)
        : TStatus(std::move(status))
        , Metadata_(std::move(metadata))
        , VolatileConfigs_(std::move(volatileConfigs))
    {}

    const std::string& GetMetadata() const {
        return Metadata_;
    }

    const std::map<ui64, std::string>& GetVolatileConfigs() const {
        return VolatileConfigs_;
    }

private:
    std::string Metadata_;
    std::map<ui64, std::string> VolatileConfigs_;
};

using TAsyncGetMetadataResult = NThreading::TFuture<TGetMetadataResult>;

struct TResolveConfigResult : public TStatus {
    TResolveConfigResult(
        TStatus&& status,
        std::string&& config)
        : TStatus(std::move(status))
        , Config_(std::move(config))
    {}

    const std::string& GetConfig() const {
        return Config_;
    }

private:
    std::string Config_;
};

using TAsyncResolveConfigResult = NThreading::TFuture<TResolveConfigResult>;

struct TGetNodeLabelsResult : public TStatus {
    TGetNodeLabelsResult(
        TStatus&& status,
        std::map<std::string, std::string>&& labels)
        : TStatus(std::move(status))
        , Labels_(std::move(labels))
    {}

    const std::map<std::string, std::string>& GetLabels() const {
        return Labels_;
    }

private:
    std::map<std::string, std::string> Labels_;
};

using TAsyncGetNodeLabelsResult = NThreading::TFuture<TGetNodeLabelsResult>;

struct TVerboseResolveConfigResult : public TStatus {
    struct TLabel {
        enum class EType {
            Negative = 0,
            Empty,
            Common,
        };

        EType Type;
        std::string Value;

        bool operator<(const TLabel& other) const {
            int lhs = static_cast<int>(Type);
            int rhs = static_cast<int>(other.Type);
            return std::tie(lhs, Value) < std::tie(rhs, other.Value);
        }

        bool operator==(const TLabel& other) const {
            int lhs = static_cast<int>(Type);
            int rhs = static_cast<int>(other.Type);
            return std::tie(lhs, Value) == std::tie(rhs, other.Value);
        }
    };

    using ConfigByLabelSet = std::map<TSet<std::vector<TLabel>>, std::string>;

    TVerboseResolveConfigResult(
        TStatus&& status,
        TSet<std::string>&& labels,
        ConfigByLabelSet&& configs)
        : TStatus(std::move(status))
        , Labels_(std::move(labels))
        , Configs_(std::move(configs))
    {}

    const TSet<std::string>& GetLabels() const {
        return Labels_;
    }

    const ConfigByLabelSet& GetConfigs() const {
        return Configs_;
    }

private:
    TSet<std::string> Labels_;
    ConfigByLabelSet Configs_;
};

using TAsyncVerboseResolveConfigResult = NThreading::TFuture<TVerboseResolveConfigResult>;


struct TDynamicConfigClientSettings : public TCommonClientSettingsBase<TDynamicConfigClientSettings> {
    using TSelf = TDynamicConfigClientSettings;
};

struct TClusterConfigSettings : public NYdb::TOperationRequestSettings<TClusterConfigSettings> {};

class TDynamicConfigClient {
public:
    class TImpl;

    explicit TDynamicConfigClient(const TDriver& driver);

    // Set config
    TAsyncStatus SetConfig(const std::string& config, bool dryRun = false, bool allowUnknownFields = false, const TClusterConfigSettings& settings = {});

    // Replace config
    TAsyncStatus ReplaceConfig(const std::string& config, bool dryRun = false, bool allowUnknownFields = false, const TClusterConfigSettings& settings = {});

    // Drop config
    TAsyncStatus DropConfig(
        const std::string& cluster,
        ui64 version,
        const TClusterConfigSettings& settings = {});

    // Add volatile config
    TAsyncStatus AddVolatileConfig(
        const std::string& config,
        const TClusterConfigSettings& settings = {});

    // Remove specific volatile configs
    TAsyncStatus RemoveVolatileConfig(
        const std::string& cluster,
        ui64 version,
        const std::vector<ui64>& ids,
        const TClusterConfigSettings& settings = {});

    // Remove all volatile config
    TAsyncStatus RemoveAllVolatileConfigs(
        const std::string& cluster,
        ui64 version,
        const TClusterConfigSettings& settings = {});

    // Remove specific volatile configs
    TAsyncStatus ForceRemoveVolatileConfig(
        const std::vector<ui64>& ids,
        const TClusterConfigSettings& settings = {});

    // Remove all volatile config
    TAsyncStatus ForceRemoveAllVolatileConfigs(
        const TClusterConfigSettings& settings = {});

    // Get current cluster configs metadata
    TAsyncGetMetadataResult GetMetadata(const TClusterConfigSettings& settings = {});

    // Get current cluster configs
    TAsyncGetConfigResult GetConfig(const TClusterConfigSettings& settings = {});

    // Get current cluster configs
    TAsyncGetNodeLabelsResult GetNodeLabels(ui64 nodeId, const TClusterConfigSettings& settings = {});

    // Resolve arbitrary config for specific labels
    TAsyncResolveConfigResult ResolveConfig(
        const std::string& config,
        const std::map<ui64, std::string>& volatileConfigs,
        const std::map<std::string, std::string>& labels,
        const TClusterConfigSettings& settings = {});

    // Resolve arbitrary config for all label combinations
    TAsyncResolveConfigResult ResolveConfig(
        const std::string& config,
        const std::map<ui64, std::string>& volatileConfigs,
        const TClusterConfigSettings& settings = {});

    // Resolve arbitrary config for all label combinations
    TAsyncVerboseResolveConfigResult VerboseResolveConfig(
        const std::string& config,
        const std::map<ui64, std::string>& volatileConfigs,
        const TClusterConfigSettings& settings = {});

private:
    std::shared_ptr<TImpl> Impl_;
};

} // namespace NYdb::NDynamicConfig
