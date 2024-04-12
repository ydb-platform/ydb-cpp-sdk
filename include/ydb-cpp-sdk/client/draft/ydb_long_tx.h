#pragma once

#include <src/api/grpc/draft/ydb_long_tx_v1.grpc.pb.h>
<<<<<<<< HEAD:include/ydb-cpp-sdk/client/draft/ydb_long_tx.h
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/types/operation/operation.h>
========
#include <src/client/ydb_driver/driver.h>
#include <src/client/ydb_types/operation/operation.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/draft/ydb_long_tx.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/draft/ydb_long_tx.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {
namespace NLongTx {

class TLongTxBeginResult : public TOperation {
public:
    explicit TLongTxBeginResult(TStatus&& status)
        : TOperation(std::move(status))
    {}

    TLongTxBeginResult(TStatus&& status, Ydb::Operations::Operation&& operation)
        : TOperation(std::move(status), std::move(operation))
    {}

    Ydb::LongTx::BeginTransactionResult GetResult() {
        Ydb::LongTx::BeginTransactionResult result;
        GetProto().result().UnpackTo(&result);
        return result;
    }
};

class TLongTxCommitResult : public TOperation {
public:
    explicit TLongTxCommitResult(TStatus&& status)
        : TOperation(std::move(status))
    {}

    TLongTxCommitResult(TStatus&& status, Ydb::Operations::Operation&& operation)
        : TOperation(std::move(status), std::move(operation))
    {}

    Ydb::LongTx::CommitTransactionResult GetResult() {
        Ydb::LongTx::CommitTransactionResult result;
        GetProto().result().UnpackTo(&result);
        return result;
    }
};

class TLongTxRollbackResult : public TOperation {
public:
    explicit TLongTxRollbackResult(TStatus&& status)
        : TOperation(std::move(status))
    {}

    TLongTxRollbackResult(TStatus&& status, Ydb::Operations::Operation&& operation)
        : TOperation(std::move(status), std::move(operation))
    {}

    Ydb::LongTx::RollbackTransactionResult GetResult() {
        Ydb::LongTx::RollbackTransactionResult result;
        GetProto().result().UnpackTo(&result);
        return result;
    }
};

class TLongTxWriteResult : public TOperation {
public:
    explicit TLongTxWriteResult(TStatus&& status)
        : TOperation(std::move(status))
    {}

    TLongTxWriteResult(TStatus&& status, Ydb::Operations::Operation&& operation)
        : TOperation(std::move(status), std::move(operation))
    {}

    Ydb::LongTx::WriteResult GetResult() {
        Ydb::LongTx::WriteResult result;
        GetProto().result().UnpackTo(&result);
        return result;
    }
};

class TLongTxReadResult : public TOperation {
public:
    explicit TLongTxReadResult(TStatus&& status)
        : TOperation(std::move(status))
    {}

    TLongTxReadResult(TStatus&& status, Ydb::Operations::Operation&& operation)
        : TOperation(std::move(status), std::move(operation))
    {}

    Ydb::LongTx::ReadResult GetResult() {
        Ydb::LongTx::ReadResult result;
        GetProto().result().UnpackTo(&result);
        return result;
    }
};

struct TClientSettings : public TCommonClientSettingsBase<TClientSettings> {
    using TSelf = TClientSettings;
};

class TClient {
public:
    using TAsyncBeginTxResult = NThreading::TFuture<TLongTxBeginResult>;
    using TAsyncCommitTxResult = NThreading::TFuture<TLongTxCommitResult>;
    using TAsyncRollbackTxResult = NThreading::TFuture<TLongTxRollbackResult>;
    using TAsyncWriteResult = NThreading::TFuture<TLongTxWriteResult>;
    using TAsyncReadResult = NThreading::TFuture<TLongTxReadResult>;

    TClient(const TDriver& driver, const TClientSettings& settings = TClientSettings());

    TAsyncBeginTxResult BeginWriteTx();
    TAsyncBeginTxResult BeginReadTx();
    TAsyncCommitTxResult CommitTx(const std::string& txId);
    TAsyncRollbackTxResult RollbackTx(const std::string& txId);
    TAsyncWriteResult Write(const std::string& txId, const std::string& table, const std::string& dedupId,
                            const std::string& data, Ydb::LongTx::Data::Format format);
    TAsyncReadResult Read(const std::string& txId, const std::string& table);

private:
    class TImpl;
    std::shared_ptr<TImpl> Impl_;
};

} // namespace NLongTx
} // namespace NYdb
