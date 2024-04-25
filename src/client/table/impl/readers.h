#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/table/impl/readers.h
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/client/resources/ydb_resources.h>

#include <src/api/grpc/ydb_table_v1.grpc.pb.h>
#include <ydb-cpp-sdk/client/proto/accessor.h>

#include <ydb-cpp-sdk/util/random/random.h>
<<<<<<< HEAD
========
#include <src/client/resources/ydb_resources.h>

#include <src/api/grpc/ydb_table_v1.grpc.pb.h>
#include <src/client/ydb_proto/accessor.h>

#include <src/util/random/random.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/readers.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_table/impl/readers.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

#include "client_session.h"
#include "data_query.h"
#include "request_migrator.h"


namespace NYdb {
namespace NTable {

using namespace NThreading;


class TTablePartIterator::TReaderImpl {
public:
    using TSelf = TTablePartIterator::TReaderImpl;
    using TResponse = Ydb::Table::ReadTableResponse;
    using TStreamProcessorPtr = NYdbGrpc::IStreamRequestReadProcessor<TResponse>::TPtr;
    using TReadCallback = NYdbGrpc::IStreamRequestReadProcessor<TResponse>::TReadCallback;
    using TGRpcStatus = NYdbGrpc::TGrpcStatus;
    using TBatchReadResult = std::pair<TResponse, TGRpcStatus>;

    TReaderImpl(TStreamProcessorPtr streamProcessor, const std::string& endpoint);
    ~TReaderImpl();
    bool IsFinished();
    TAsyncSimpleStreamPart<TResultSet> ReadNext(std::shared_ptr<TSelf> self);

private:
    TStreamProcessorPtr StreamProcessor_;
    TResponse Response_;
    bool Finished_;
    std::string Endpoint_;
};


class TScanQueryPartIterator::TReaderImpl {
public:
    using TSelf = TScanQueryPartIterator::TReaderImpl;
    using TResponse = Ydb::Table::ExecuteScanQueryPartialResponse;
    using TStreamProcessorPtr = NYdbGrpc::IStreamRequestReadProcessor<TResponse>::TPtr;
    using TReadCallback = NYdbGrpc::IStreamRequestReadProcessor<TResponse>::TReadCallback;
    using TGRpcStatus = NYdbGrpc::TGrpcStatus;
    using TBatchReadResult = std::pair<TResponse, TGRpcStatus>;

    TReaderImpl(TStreamProcessorPtr streamProcessor, const std::string& endpoint);
    ~TReaderImpl();
    bool IsFinished() const;
    TAsyncScanQueryPart ReadNext(std::shared_ptr<TSelf> self);

private:
    TStreamProcessorPtr StreamProcessor_;
    TResponse Response_;
    bool Finished_;
    std::string Endpoint_;
};


}
}
