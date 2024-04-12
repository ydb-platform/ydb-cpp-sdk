#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/extensions/discovery_mutator/discovery_mutator.h
#include <ydb-cpp-sdk/client/ydb_extension/extension.h>
========
#include <src/client/ydb_extension/extension.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/extensions/discovery_mutator/discovery_mutator.h

namespace NDiscoveryMutator {

class TDiscoveryMutator: public NYdb::IExtension {
public:
    using IApi = NYdb::IDiscoveryMutatorApi;

    using TCb = NYdb::IDiscoveryMutatorApi::TMutatorCb;

    class TParams {
        friend class TDiscoveryMutator;
    public:
        TParams(TCb mutator)
            : Mutator_(std::move(mutator))
        { }

    private:
        TCb Mutator_;
    };

    TDiscoveryMutator(TParams params, IApi* api) {
        api->SetMutatorCb(std::move(params.Mutator_));
    }
};

}; // namespace NDiscoveryModifie
