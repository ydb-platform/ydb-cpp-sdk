#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/client/query/stats.h
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_query/stats.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_query/stats.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/datetime/base.h>
>>>>>>> origin/main

#include <memory>
#include <optional>
#include <string>

class TDuration;

namespace Ydb::TableStats {
    class QueryStats;
}

namespace NYdb {
    class TProtoAccessor;
}

namespace NYdb::NQuery {

class TExecStats {
    friend class NYdb::TProtoAccessor;

public:
    explicit TExecStats(Ydb::TableStats::QueryStats&& proto);
    explicit TExecStats(const Ydb::TableStats::QueryStats& proto);

    std::string ToString(bool withPlan = false) const;

    std::optional<std::string> GetPlan() const;

    TDuration GetTotalDuration() const;
    TDuration GetTotalCpuTime() const;

private:
    const Ydb::TableStats::QueryStats& GetProto() const;

private:
    class TImpl;
    std::shared_ptr<TImpl> Impl_;
};

} // namespace NYdb::NQuery
