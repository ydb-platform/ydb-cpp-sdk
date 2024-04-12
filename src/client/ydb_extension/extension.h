#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/ydb_extension/extension.h
#include <ydb-cpp-sdk/client/driver/driver.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_registry.h>
========
#include <src/client/ydb_driver/driver.h>

#include <src/library/monlib/metrics/metric_registry.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_extension/extension.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_extension/extension.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace Ydb {
namespace Discovery {

class ListEndpointsResult;

}
}

namespace NYdb {

class IExtensionApi {
public:
    friend class TDriver;
public:
    virtual ~IExtensionApi() = default;
private:
    void SelfRegister(TDriver driver);
};

class IExtension {
    friend class TDriver;
public:
    virtual ~IExtension() = default;
private:
    void SelfRegister(TDriver driver);
};

namespace NSdkStats {

class IStatApi: public IExtensionApi {
public:
    static IStatApi* Create(TDriver driver);
public:
    virtual void SetMetricRegistry(::NMonitoring::IMetricRegistry* sensorsRegistry) = 0;
    virtual void Accept(::NMonitoring::IMetricConsumer* consumer) const = 0;
};

class DestroyedClientException: public yexception {};

} // namespace NSdkStats


class IDiscoveryMutatorApi: public IExtensionApi {
public:
    struct TAuxInfo {
        std::string_view Database;
        std::string_view DiscoveryEndpoint;
    };
    using TMutatorCb = std::function<TStatus(Ydb::Discovery::ListEndpointsResult* proto, TStatus status, const TAuxInfo& aux)>;

    static IDiscoveryMutatorApi* Create(TDriver driver);
public:
    virtual void SetMutatorCb(TMutatorCb&& mutator) = 0;
};


template<typename TExtension>
void TDriver::AddExtension(typename TExtension::TParams params) {
    typename TExtension::IApi* api = TExtension::IApi::Create(*this);
    auto extension = new TExtension(params, api);
    extension->SelfRegister(*this);
    if (api)
        api->SelfRegister(*this);
}

} // namespace NYdb
