#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>

namespace Ydb {
namespace Discovery {
    class ListEndpointsResult;
    class WhoAmIResult;
    class NodeRegistrationResult;
    class NodeLocation;
    class NodeInfo;
} // namespace Discovery
} // namespace Ydb

namespace NYdb {
namespace NDiscovery {

////////////////////////////////////////////////////////////////////////////////

class TListEndpointsSettings : public TSimpleRequestSettings<TListEndpointsSettings> {};

struct TWhoAmISettings : public TSimpleRequestSettings<TWhoAmISettings> {
    FLUENT_SETTING_DEFAULT(bool, WithGroups, false);
};

struct TNodeLocation {
    TNodeLocation() = default;
    TNodeLocation(const Ydb::Discovery::NodeLocation& location);

    std::optional<ui32> DataCenterNum;
    std::optional<ui32> RoomNum;
    std::optional<ui32> RackNum;
    std::optional<ui32> BodyNum;
    std::optional<ui32> Body;

    std::optional<std::string> DataCenter;
    std::optional<std::string> Module;
    std::optional<std::string> Rack;
    std::optional<std::string> Unit;
};

struct TNodeRegistrationSettings : public TSimpleRequestSettings<TNodeRegistrationSettings> {
    FLUENT_SETTING(std::string, Host);
    FLUENT_SETTING(ui32, Port);
    FLUENT_SETTING(std::string, ResolveHost);
    FLUENT_SETTING(std::string, Address);
    FLUENT_SETTING(TNodeLocation, Location);
    FLUENT_SETTING(std::string, DomainPath);
    FLUENT_SETTING_DEFAULT(bool, FixedNodeId, false);
    FLUENT_SETTING(std::string, Path);
};

struct TEndpointInfo {
    std::string Address;
    ui32 Port = 0;
    float LoadFactor = 0.0;
    bool Ssl = false;
    std::vector<std::string> Services;
    std::string Location;
    ui32 NodeId = 0;
    std::vector<std::string> IPv4Addrs;
    std::vector<std::string> IPv6Addrs;
    std::string SslTargetNameOverride;
};

class TListEndpointsResult : public TStatus {
public:
    TListEndpointsResult(TStatus&& status, const Ydb::Discovery::ListEndpointsResult& endpoints);
    const std::vector<TEndpointInfo>& GetEndpointsInfo() const;
private:
    std::vector<TEndpointInfo> Info_;
};

using TAsyncListEndpointsResult = NThreading::TFuture<TListEndpointsResult>;

class TWhoAmIResult : public TStatus {
public:
    TWhoAmIResult(TStatus&& status, const Ydb::Discovery::WhoAmIResult& proto);
    const std::string& GetUserName() const;
    const std::vector<std::string>& GetGroups() const;
private:
    std::string UserName_;
    std::vector<std::string> Groups_;
};

using TAsyncWhoAmIResult = NThreading::TFuture<TWhoAmIResult>;

struct TNodeInfo {
    TNodeInfo() = default;
    TNodeInfo(const Ydb::Discovery::NodeInfo& info);

    ui32 NodeId;
    std::string Host;
    ui32 Port;
    std::string ResolveHost;
    std::string Address;
    TNodeLocation Location;
    ui64 Expire;
};

class TNodeRegistrationResult : public TStatus {
public:
    TNodeRegistrationResult() : TStatus(EStatus::GENERIC_ERROR, NYql::TIssues()) {}
    TNodeRegistrationResult(TStatus&& status, const Ydb::Discovery::NodeRegistrationResult& proto);
    const ui32& GetNodeId() const;
    const std::string& GetDomainPath() const;
    const ui64& GetExpire() const;
    const ui64& GetScopeTabletId() const;
    bool HasScopeTabletId() const;
    const ui64& GetScopePathId() const;
    bool HasScopePathId() const;
    const std::string& GetSlotName() const;
    const std::vector<TNodeInfo>& GetNodes() const;

private:
    ui32 NodeId_;
    std::string DomainPath_;
    ui64 Expire_;
    std::optional<ui64> ScopeTableId_;
    std::optional<ui64> ScopePathId_;
    std::string SlotName_;
    std::vector<TNodeInfo> Nodes_;
};

using TAsyncNodeRegistrationResult = NThreading::TFuture<TNodeRegistrationResult>;

////////////////////////////////////////////////////////////////////////////////

class TDiscoveryClient {
public:
    explicit TDiscoveryClient(const TDriver& driver, const TCommonClientSettings& settings = TCommonClientSettings());

    TAsyncListEndpointsResult ListEndpoints(const TListEndpointsSettings& settings = TListEndpointsSettings());
    TAsyncWhoAmIResult WhoAmI(const TWhoAmISettings& settings = TWhoAmISettings());
    TAsyncNodeRegistrationResult NodeRegistration(const TNodeRegistrationSettings& settings = TNodeRegistrationSettings());

private:
    class TImpl;
    std::shared_ptr<TImpl> Impl_;
};

} // namespace NTable
} // namespace NDiscovery
