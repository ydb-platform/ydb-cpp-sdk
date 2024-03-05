#pragma once

#include <util/datetime/base.h>

#include <string>

#include <memory>

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
