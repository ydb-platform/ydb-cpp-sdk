#pragma once

#include <ydb-cpp-sdk/client/types/ydb.h>

#include <src/client/impl/internal/internal_header.h>

#include <memory>
#include <unordered_map>

namespace NYdb::inline V3 {

class TBalancingPolicy::TImpl {
public:
    enum class EPolicyType {
        UseAllNodes,
        UsePreferableLocation,
        UsePreferablePileState
    };

    static TImpl UseAllNodes();

    static TImpl UsePreferableLocation(const std::string& location);

    static TImpl UsePreferablePileState(EPileState pileState);

    EPolicyType PolicyType;

    // UsePreferableLocation
    std::string Location;

    // UsePreferablePileState
    EPileState PileState;
};

}
