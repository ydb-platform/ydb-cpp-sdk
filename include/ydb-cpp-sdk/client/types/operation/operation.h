#pragma once

#include <ydb-cpp-sdk/library/operation_id/operation_id.h>

#include <ydb-cpp-sdk/library/threading/future/future.h>

#include <google/protobuf/stubs/status.h>
#include <google/protobuf/util/json_util.h>

namespace Ydb {
namespace Operations {

class Operation;

} // namespace Operations
} // namespace Ydb

namespace NYdb {

class TStatus;

class TOperation {
public:
    using TOperationId = NKikimr::NOperationId::TOperationId;

public:
    TOperation(TStatus&& status);
    TOperation(TStatus&& status, Ydb::Operations::Operation&& operation);
    virtual ~TOperation() = default;

    const TOperationId& Id() const;
    bool Ready() const;
    const TStatus& Status() const;

    std::string ToString() const;
    std::string ToJsonString() const;
    void Out(IOutputStream& o) const;

protected:
    const Ydb::Operations::Operation& GetProto() const;

private:
    class TImpl;
    std::shared_ptr<TImpl> Impl_;
};

using TAsyncOperation = NThreading::TFuture<TOperation>;

} // namespace NYdb
