#pragma once

#include <ydb-cpp-sdk/client/result/result.h>

namespace NYdb::inline V3 {

//! Provides access to Arrow batches of result set. It is not recommended to use this
//! class in client applications as it is an experimental feature.
class TArrowAccessor {
public:
    static TResultSet::EFormat Format(const TResultSet& resultSet);
    static const std::string& GetArrowSchema(const TResultSet& resultSet);
    static const std::vector<std::string>& GetArrowBatches(const TResultSet& resultSet);
};

} // namespace NYdb
