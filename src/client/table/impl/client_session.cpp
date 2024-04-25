#include "client_session.h"
#include "data_query.h"

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/table/impl/client_session.cpp
#include <ydb-cpp-sdk/util/string/cast.h>
========
#include <src/util/string/cast.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/client_session.cpp
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_table/impl/client_session.cpp
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/cast.h>
>>>>>>> origin/main

namespace NYdb {
namespace NTable {

TSession::TImpl::TImpl(const std::string& sessionId, const std::string& endpoint, bool useQueryCache, ui32 queryCacheSize, bool isOwnedBySessionPool)
    : TKqpSessionCommon(sessionId, endpoint, isOwnedBySessionPool)
    , UseQueryCache_(useQueryCache)
    , QueryCache_(queryCacheSize)
{}

void TSession::TImpl::InvalidateQueryInCache(const std::string& key) {
    if (!UseQueryCache_) {
        return;
    }

    std::lock_guard guard(Lock_);
    auto it = QueryCache_.Find(key);
    if (it != QueryCache_.End()) {
        QueryCache_.Erase(it);
    }
}

void TSession::TImpl::InvalidateQueryCache() {
    if (!UseQueryCache_) {
        return;
    }

    std::lock_guard guard(Lock_);
    QueryCache_.Clear();
}

std::optional<TSession::TImpl::TDataQueryInfo> TSession::TImpl::GetQueryFromCache(const std::string& query, bool allowMigration) {
    if (!UseQueryCache_) {
        return {};
    }

    auto key = EncodeQuery(query, allowMigration);

    std::lock_guard guard(Lock_);
    auto it = QueryCache_.Find(key);
    if (it != QueryCache_.End()) {
        return *it;
    }

    return std::nullopt;
}

void TSession::TImpl::AddQueryToCache(const TDataQuery& query) {
    if (!UseQueryCache_) {
        return;
    }

    const auto& id = query.Impl_->GetId();
    if (id.empty()) {
        return;
    }

    auto key = query.Impl_->GetTextHash();
    TDataQueryInfo queryInfo(id, query.Impl_->GetParameterTypes());

    std::lock_guard guard(Lock_);
    auto it = QueryCache_.Find(key);
    if (it != QueryCache_.End()) {
        *it = queryInfo;
    } else {
        QueryCache_.Insert(key, queryInfo);
    }
}

const TLRUCache<std::string, TSession::TImpl::TDataQueryInfo>& TSession::TImpl::GetQueryCacheUnsafe() const {
    return QueryCache_;
}

} // namespace NTable
} // namespace NYdb
