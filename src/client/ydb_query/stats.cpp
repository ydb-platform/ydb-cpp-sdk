#include <ydb-cpp-sdk/client/query/stats.h>

#include <src/api/protos/ydb_table.pb.h>

<<<<<<<< HEAD:src/client/query/stats.cpp
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/datetime/base.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_query/stats.cpp

#include <google/protobuf/text_format.h>

namespace NYdb::NQuery {

class TExecStats::TImpl {
public:
    Ydb::TableStats::QueryStats Proto;
};

TExecStats::TExecStats(const Ydb::TableStats::QueryStats& proto) {
    Impl_ = std::make_shared<TImpl>();
    Impl_->Proto = proto;
}

TExecStats::TExecStats(Ydb::TableStats::QueryStats&& proto) {
    Impl_ = std::make_shared<TImpl>();
    Impl_->Proto = std::move(proto);
}

std::string TExecStats::ToString(bool withPlan) const {
    auto proto = Impl_->Proto;

    if (!withPlan) {
        proto.clear_query_plan();
        proto.clear_query_ast();
    }

    std::string res;
    ::google::protobuf::TextFormat::PrintToString(proto, &res);
    return res;
}

std::optional<std::string> TExecStats::GetPlan() const {
    auto proto = Impl_->Proto;

    if (proto.query_plan().empty()) {
        return {};
    }

    return proto.query_plan();
}

TDuration TExecStats::GetTotalDuration() const {
    return TDuration::MicroSeconds(Impl_->Proto.total_duration_us());
}

TDuration TExecStats::GetTotalCpuTime() const {
    return TDuration::MicroSeconds(Impl_->Proto.total_cpu_time_us());
}

const Ydb::TableStats::QueryStats& TExecStats::GetProto() const {
    return Impl_->Proto;
}

} // namespace NYdb::NQuery
