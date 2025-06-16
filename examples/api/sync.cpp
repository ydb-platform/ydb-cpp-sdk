#include <ydb-cpp-sdk/client/types/fluent_settings_helpers.h>
#include <ydb-cpp-sdk/client/params/params.h>
#include <ydb-cpp-sdk/client/types/status/status.h>
#include <ydb-cpp-sdk/client/types/request_settings.h>

#include <string>
#include <functional>
#include <chrono>

using namespace NYdb;
using namespace NYdb::NStatusHelpers;
using namespace std::chrono_literals;

struct TClientSettings {};
struct TDriver {};

struct TRow {};

struct TRowIterator {
    TRowIterator& operator++();

    TRow& operator*() const;
    TRow* operator->() const;

    bool operator==(const TRowIterator& other) const;
};

struct TRowsStream {
    TRowIterator begin() const {
        return TRowIterator();
    }

    TRowIterator end() const {
        return TRowIterator();
    }
};

struct TResultsStream {
    TRowIterator begin() const {
        return TRowIterator();
    }

    TRowIterator end() const {
        return TRowIterator();
    }

    TRowsStream NextResultSet() {
        return TRowsStream();
    }
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
    TResultsStream ExecuteQuery(const std::string& query, const TQueryTxOpts& opts = {});

    void Commit(const TCommitOpts& opts = {});
    void Rollback(const TRollbackOpts& opts = {});
};

class TQueryClient {
public:
    TQueryClient(const TDriver& driver, const TClientSettings& settings = {}) {}

    void Retry(std::function<void(TTransaction& tx)>&& txFunc, const TRetryOpts& opts = {}) {}
};

int main() {    
    TQueryClient client(TDriver{}, TClientSettings{});

    // 1. Выполнение интерактивной транзакции

    try {
        client.Retry([](TTransaction& tx) {
            auto result1 = tx.ExecuteQuery(R"(
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
            for (const auto& row : result1.NextResultSet()) {
                // parse SELECT id, data FROM table_1
            }
            for (const auto& row : result1.NextResultSet()) {
                // parse SELECT id FROM table_2
            }
            auto opts = TQueryTxOpts()
                .AutoCommit()
                .ClientTimeout(10ms);
            for (const auto& row : tx.ExecuteQuery("SELECT * FROM table", opts)) {
                // parse SELECT * FROM table
            }
        }, TRetryOpts()
            .TxSettings(TTxSettings::SerializableRW())
            .ClientTimeout(1s)
            .Idempotent()
        );
    } catch (const TYdbErrorException& e) {
        std::cerr << e.what() << std::endl;
    }

    // 2. libpqxx

    try {
        pqxx::connection conn("dbname=mydb connect_timeout=1");
        pqxx::work txn(conn, "interactive_tx_example", pqxx::isolation_level::serializable);
        
        // Нет обработки нескольких резалт сетов
        pqxx::result result1 = txn.exec_params(
            R"(
                INSERT INTO small_table(id, data) VALUES($1, $2);
                SELECT id, data FROM table_1;
            )",
            1,                 // $1: id
            "Some text"        // $2: data
        );
        for (const auto& row : result1) {
            // Обработка строки
        }

        pqxx::result result2 = txn.exec("SELECT id FROM table_2;");
        for (const auto& row : result2) {
            // Обработка строки
        }
        pqxx::result result3 = txn.exec("SELECT * FROM table");
        
        for (const auto& row : result3) {
            // Обработка строки
        }
        
        txn.commit();
    } catch (const pqxx::sql_error& e) {
        std::cerr << e.what() << std::endl;
    }
}
