#pragma once

#include "params.h"

namespace NYdb {

class TParams::TImpl {
public:
    TImpl(::google::protobuf::Map<TString, Ydb::TypedValue>&& paramsMap);

    bool Empty() const;
    std::map<TString, TValue> GetValues() const;
    std::optional<TValue> GetValue(const TString& name) const;
    ::google::protobuf::Map<TString, Ydb::TypedValue>* GetProtoMapPtr();
    const ::google::protobuf::Map<TString, Ydb::TypedValue>& GetProtoMap() const;

private:
    ::google::protobuf::Map<TString, Ydb::TypedValue> ParamsMap_;
};

} // namespace NYdb
