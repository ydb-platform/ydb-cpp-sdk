#include <ydb-cpp-sdk/client/types/fluent_settings_helpers.h>
#include <ydb-cpp-sdk/client/params/params.h>
#include <ydb-cpp-sdk/client/types/status/status.h>
#include <ydb-cpp-sdk/client/types/request_settings.h>

#include <library/cpp/threading/future/core/coroutine_traits.h>

#include <string>
#include <functional>
#include <chrono>

using namespace NYdb;
using namespace NYdb::NStatusHelpers;
using namespace NThreading;
using namespace std::chrono_literals;

struct TClientSettings {};
struct TDriver {};

struct TRow {};

struct TRowsIterator {
    TFuture<TRow> Next();

    bool IsEnd() const;
};

struct TStreamIterator : public TRowsIterator {
    TFuture<TRowsIterator> NextResultSet();
};

struct TTxOnlineSettings {
    using TSelf = TTxOnlineSettings;

    FLUENT_SETTING_DEFAULT(bool, AllowInconsistentReads, false);

    TTxOnlineSettings() {}
};

struct TTxSettings {
    using TSelf = TTxSettings;

    TTxSettings()
        : Mode_(TS_NO_TX) {}

    static TTxSettings SerializableRW() {
        return TTxSettings(TS_SERIALIZABLE_RW);
    }

    static TTxSettings OnlineRO(const TTxOnlineSettings& settings = TTxOnlineSettings()) {
        return TTxSettings(TS_ONLINE_RO).OnlineSettings(settings);
    }

    static TTxSettings StaleRO() {
        return TTxSettings(TS_STALE_RO);
    }

    static TTxSettings SnapshotRO() {
        return TTxSettings(TS_SNAPSHOT_RO);
    }

    static TTxSettings SnapshotRW() {
        return TTxSettings(TS_SNAPSHOT_RW);
    }

    static TTxSettings NoTx() {
        return TTxSettings(TS_NO_TX);
    }

private:
    enum ETransactionMode {
        TS_SERIALIZABLE_RW,
        TS_ONLINE_RO,
        TS_STALE_RO,
        TS_SNAPSHOT_RO,
        TS_SNAPSHOT_RW,
        TS_NO_TX,
    };

    FLUENT_SETTING(TTxOnlineSettings, OnlineSettings);

private:
    TTxSettings(ETransactionMode mode)
        : Mode_(mode) {}

    ETransactionMode Mode_;
};

template<typename Derived>
struct TRetryOptsCommon : public TRequestSettings<Derived> {
    using TSelf = Derived;

    FLUENT_SETTING(TTxSettings, TxSettings);

    FLUENT_SETTING_FLAG(Idempotent);
    //..
};

struct TRetryOpts : public TRetryOptsCommon<TRetryOpts> {};

template<typename Derived>
struct TQueryOptsCommon {
    using TSelf = Derived;

    FLUENT_SETTING_DEFAULT(TParams, Params, TParamsBuilder().Build());
    // ...
};

struct TQueryOpts : public TQueryOptsCommon<TQueryOpts>, public TRetryOptsCommon<TQueryOpts> {};

struct TQueryTxOpts : public TQueryOptsCommon<TQueryTxOpts>, public TRequestSettings<TQueryTxOpts> {
    FLUENT_SETTING_FLAG(AutoCommit);
};

struct TCommitOpts : public TRequestSettings<TCommitOpts> {};
struct TRollbackOpts : public TRequestSettings<TRollbackOpts> {};

class TTransaction {
public:
    TFuture<TStreamIterator> ExecuteQuery(const std::string& query, const TQueryTxOpts& opts = {});

    TFuture<void> Commit(const TCommitOpts& opts = {});
    TFuture<void> Rollback(const TRollbackOpts& opts = {});
};

class TQueryClient {
public:
    TQueryClient(const TDriver& driver, const TClientSettings& settings = {}) {}

    TFuture<void> Retry(std::function<TFuture<void>(TTransaction)>&& txFunc, const TRetryOpts& opts = {});
};

// Examples

// 1. Выполнение интерактивной транзакции
template<typename T>
TFuture<void> ProcessRowsAsync(T rowsIt) {
    if (rowsIt.IsEnd()) {
        return MakeFuture<void>();
    }

    return rowsIt.Next().Apply([rowsIt](auto future) mutable {
        auto row = future.GetValue();
        // parse row
        return ProcessRowsAsync(rowsIt);
    });
}

TFuture<void> Example1(TQueryClient& client) {
    return client.Retry([](TTransaction tx) {
        std::shared_ptr<TStreamIterator> streamIt;
        return tx.ExecuteQuery(R"(
            UPSERT INTO table (id, data) VALUES ($id, $data);
            SELECT id, data FROM table_1;
            SELECT id FROM table_2;
        )", TQueryTxOpts()
            .ClientTimeout(10ms)
            .Params(TParamsBuilder()
                .AddParam("id")
                    .Uint64(1)
                    .Build()
                .AddParam("data")
                    .String("Some text")
                    .Build()
                .Build()))
        .Apply([streamIt](auto future) mutable {
            *streamIt = future.GetValue();
            return streamIt->NextResultSet();
        })
        .Apply([](auto future) {
            // parse SELECT id, data FROM table_1
            return ProcessRowsAsync(future.GetValue());
        })
        .Apply([streamIt](auto future) mutable {
            return streamIt->NextResultSet();
        })
        .Apply([](auto future) {
            // parse SELECT id FROM table_2
            return ProcessRowsAsync(future.GetValue());
        })
        .Apply([tx](auto future) mutable {
            future.TryRethrow();
            return tx.ExecuteQuery("SELECT * FROM table", TQueryTxOpts()
                .AutoCommit()
                .ClientTimeout(10ms));
        })
        .Apply([](auto future) {
            // parse SELECT * FROM table
            return ProcessRowsAsync(future.GetValue());
        });
    }, TRetryOpts()
        .TxSettings(TTxSettings::SerializableRW())
        .ClientTimeout(1s)
        .Idempotent());
}

// 1.2 Выполнение интерактивной транзакции (корутины)
TFuture<void> ExampleCoro1(TQueryClient& client) {
    co_return co_await client.Retry([](TTransaction tx) -> TFuture<void> {
        auto result1 = co_await tx.ExecuteQuery(R"(
            UPSERT INTO table (id, data) VALUES ($id, $data);
            SELECT id, data FROM table_1;
            SELECT id FROM table_2;
        )", TQueryTxOpts()
            .ClientTimeout(10ms)
            .Params(TParamsBuilder()
                .AddParam("id")
                    .Uint64(1)
                    .Build()
                .AddParam("data")
                    .String("Some text")
                    .Build()
                .Build())
        );
        auto rowsIt = co_await result1.NextResultSet();
        while (!rowsIt.IsEnd()) {
            auto row = co_await rowsIt.Next();
            // parse SELECT id, data FROM table_1
        }
        auto rowsIt2 = co_await result1.NextResultSet();
        while (!rowsIt2.IsEnd()) {
            auto row = co_await rowsIt2.Next();
            // parse SELECT id FROM table_2
        }
        auto result2 = co_await tx.ExecuteQuery("SELECT * FROM table", TQueryTxOpts()
            .AutoCommit()
            .ClientTimeout(10ms));
        while (!result2.IsEnd()) {
            auto row = co_await result2.Next();
            // parse SELECT * FROM table
        }
    }, TRetryOpts()
        .TxSettings(TTxSettings::SerializableRW())
        .ClientTimeout(1s)
        .Idempotent());
}

// 1.3 Выполнение интерактивной транзакции (boost::asio)
boost::asio::awaitable<void> ExampleCoroBoost1(boost::mysql::any_connection& conn) {
    boost::mysql::any_connection conn(co_await asio::this_coro::executor);

    boost::mysql::connect_params params;
    co_await conn.async_connect(params);

    const char* query = R"(
        START TRANSACTION;
        UPSERT INTO table (id, data) VALUES ({}, {});
        SELECT id, data FROM table_1;
        SELECT id FROM table_2;
        SELECT * FROM table;
        COMMIT;
    )";
    
    boost::mysql::results result;
    co_await conn.async_execute(boost::mysql::with_params(query, 1, "Some text"), result);

    for (const auto& row : result.at(0).rows()) {
        // parse SELECT id, data FROM table_1
    }
    for (const auto& row : result.at(1).rows()) {
        // parse SELECT id FROM table_2
    }
    for (const auto& row : result.at(2).rows()) {
        // parse SELECT * FROM table
    }

    co_await conn.async_close();
}

int main() {
    TQueryClient client{TDriver(), TClientSettings()};
    Example1(client).Wait();
    ExampleCoro1(client).Wait();
    return 0;
}
