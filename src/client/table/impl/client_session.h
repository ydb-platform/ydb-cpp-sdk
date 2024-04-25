#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/table/impl/client_session.h
=======
>>>>>>> origin/main
#include <ydb-cpp-sdk/client/table/table.h>
#include <src/client/impl/ydb_internal/kqp_session_common/kqp_session_common.h>
#include <src/client/impl/ydb_endpoints/endpoints.h>
#include <ydb-cpp-sdk/library/operation_id/operation_id.h>
<<<<<<< HEAD
========
#include <src/client/ydb_table/table.h>
#include <src/client/impl/ydb_internal/kqp_session_common/kqp_session_common.h>
#include <src/client/impl/ydb_endpoints/endpoints.h>
#include <src/library/operation_id/operation_id.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/client_session.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_table/impl/client_session.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main

#include <src/api/protos/ydb_table.pb.h>

#include <src/library/cache/cache.h>

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/table/impl/client_session.h
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/client_session.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_table/impl/client_session.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/datetime/base.h>
>>>>>>> origin/main

#include <functional>

namespace NYdb {
namespace NTable {

////////////////////////////////////////////////////////////////////////////////

using TSessionInspectorFn = std::function<void(TAsyncCreateSessionResult future)>;


class TSession::TImpl : public TKqpSessionCommon {
    friend class TTableClient;
    friend class TSession;

#ifdef YDB_IMPL_TABLE_CLIENT_SESSION_UT
public:
#endif
    TImpl(const std::string& sessionId, const std::string& endpoint, bool useQueryCache, ui32 queryCacheSize, bool isOwnedBySessionPool);
public:
    struct TDataQueryInfo {
        std::string QueryId;
        ::google::protobuf::Map<std::string, Ydb::Type> ParameterTypes;

        TDataQueryInfo() {}

        TDataQueryInfo(const std::string& queryId,
            const ::google::protobuf::Map<std::string, Ydb::Type>& parameterTypes)
            : QueryId(queryId)
            , ParameterTypes(parameterTypes) {}
    };
public:
    ~TImpl() = default;

    void InvalidateQueryInCache(const std::string& key);
    void InvalidateQueryCache();
    std::optional<TDataQueryInfo> GetQueryFromCache(const std::string& query, bool allowMigration);
    void AddQueryToCache(const TDataQuery& query);

    const TLRUCache<std::string, TDataQueryInfo>& GetQueryCacheUnsafe() const;

    static TSessionInspectorFn GetSessionInspector(
        NThreading::TPromise<TCreateSessionResult>& promise,
        std::shared_ptr<TTableClient::TImpl> client,
        const TCreateSessionSettings& settings,
        ui32 counter, bool needUpdateActiveSessionCounter);

private:
    bool UseQueryCache_;
    TLRUCache<std::string, TDataQueryInfo> QueryCache_;
};

} // namespace NTable
} // namespace NYdb
