#include "connection.h"
#include "environment.h"
#include "descriptor.h"
#include "statement.h"

#include <gtest/gtest.h>

#include <util/generic/scope.h>

#include <barrier>
#include <array>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

using namespace std::chrono_literals;
using namespace NYdb::NOdbc;

namespace {

class TConnectionHandles {
public:
    SQLHENV Env = SQL_NULL_HENV;
    SQLHDBC Dbc = SQL_NULL_HDBC;

    TConnectionHandles() {
        EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &Env), SQL_SUCCESS);
        EXPECT_EQ(SQLSetEnvAttr(Env, SQL_ATTR_ODBC_VERSION,
                               reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0), SQL_SUCCESS);
        EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, Env, &Dbc), SQL_SUCCESS);
        EXPECT_EQ(SQLDriverConnect(Dbc, nullptr,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>("Server=localhost:2136;Database=/local;")),
            SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT), SQL_SUCCESS);
    }

    ~TConnectionHandles() {
        if (Dbc) {
            SQLDisconnect(Dbc);
            SQLFreeHandle(SQL_HANDLE_DBC, Dbc);
        }
        if (Env) {
            SQLFreeHandle(SQL_HANDLE_ENV, Env);
        }
    }
};

class TStatementHandle {
public:
    SQLHSTMT Handle = SQL_NULL_HSTMT;

    explicit TStatementHandle(SQLHDBC dbc) {
        EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &Handle), SQL_SUCCESS);
        EXPECT_EQ(SQLSetStmtAttr(Handle, SQL_ATTR_QUERY_TIMEOUT,
                               reinterpret_cast<SQLPOINTER>(10), 0), SQL_SUCCESS);
    }

    explicit TStatementHandle(std::unique_ptr<TStatement> statement) {
        auto& connection = statement->GetConnection();
        {
            auto lifecycle = connection.LockShared();
            auto operation = connection.LockOperation();
            connection.AddChild(statement.get());
            Handle = static_cast<THandle*>(statement.release());
        }
        EXPECT_EQ(SQLSetStmtAttr(Handle, SQL_ATTR_QUERY_TIMEOUT,
                               reinterpret_cast<SQLPOINTER>(10), 0), SQL_SUCCESS);
    }

    ~TStatementHandle() {
        if (Handle) {
            SQLFreeHandle(SQL_HANDLE_STMT, Handle);
        }
    }
};

SQLRETURN Execute(SQLHSTMT stmt, const char* query = "SELECT 42 AS value") {
    return SQLExecDirect(stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>(query)), SQL_NTS);
}

std::string SqlState(SQLHANDLE handle, SQLSMALLINT type = SQL_HANDLE_STMT) {
    SQLCHAR state[6] = {};
    SQLGetDiagRec(type, handle, 1, state, nullptr, nullptr, 0, nullptr);
    return reinterpret_cast<char*>(state);
}

void CancelActiveExecution(SQLHSTMT stmt, std::future<SQLRETURN>& execution) {
    auto cancellation = std::async(std::launch::async, [&] {
        auto owner = CheckedHandle<TStatement>(stmt);
        EXPECT_TRUE(owner->IsExecuting());
        EXPECT_EQ(execution.wait_for(0s), std::future_status::timeout);
        const auto result = SQLCancel(stmt);
        return result;
    });
    EXPECT_EQ(cancellation.wait_for(1s), std::future_status::ready);
    EXPECT_EQ(cancellation.get(), SQL_SUCCESS);
}

template<class Predicate>
bool WaitUntil(Predicate predicate) {
    const auto deadline = std::chrono::steady_clock::now() + 5s;
    do {
        if (predicate()) {
            return true;
        }
        std::this_thread::yield();
    } while (std::chrono::steady_clock::now() < deadline);
    return false;
}

template<class Call>
void CancelWhileWaitingForDescriptor(SQLHSTMT handle, SQLINTEGER attribute, Call call) {
    SQLHDESC descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(handle, attribute, &descriptor, sizeof(descriptor), nullptr), SQL_SUCCESS);
    auto* statement = CheckedHandle<TStatement>(handle);
    // Destroy the descriptor lock before the futures on every exit path.
    std::future<SQLRETURN> execution, cancellation;
    auto operation = CheckedHandle<TDescriptor>(descriptor)->LockOperation();
    execution = std::async(std::launch::async, call);
    EXPECT_TRUE(WaitUntil([&] { return statement->IsExecuting(); }));
    EXPECT_EQ(execution.wait_for(0s), std::future_status::timeout);
    cancellation = std::async(std::launch::async, [&] { return SQLCancel(handle); });
    const auto ready = cancellation.wait_for(1s);
    EXPECT_EQ(ready, std::future_status::ready);
    if (ready == std::future_status::ready) {
        EXPECT_TRUE(statement->IsCancelRequested());
    }
    operation.unlock();
    EXPECT_EQ(cancellation.get(), SQL_SUCCESS);
    const auto result = execution.get();
    if (result == SQL_ERROR) {
        EXPECT_EQ(SqlState(handle), "HY008");
    } else {
        EXPECT_EQ(result, SQL_SUCCESS) << SqlState(handle);
    }
    EXPECT_EQ(SQLFetch(handle), SQL_NO_DATA);
}

class TPausedStatement : public TStatement {
public:
    using TStatement::TStatement;

    void PauseNextExecution() {
        State_ = EState::Armed;
    }

    bool WaitUntilPaused() {
        return WaitUntil([&] { return State_ == EState::Paused; });
    }

    void Release() {
        State_ = EState::Idle;
    }

protected:
    NYdb::NQuery::TExecuteQueryResult ExecuteQuery(
        NYdb::NQuery::TSession& session, const NYdb::TParams& params, SQLULEN paramSet) override
    {
        auto result = TStatement::ExecuteQuery(session, params, paramSet);
        auto expected = EState::Armed;
        if (State_.compare_exchange_strong(expected, EState::Paused)) {
            EXPECT_TRUE(result.IsSuccess()) << result.GetIssues().ToString();
            // Keep the ODBC call active before processing the real query result.
            if (!WaitUntil([&] { return State_ == EState::Idle; })) {
                throw TOdbcException("HY000", 0, "Timed out waiting to release the test query result");
            }
        }
        return result;
    }

private:
    enum class EState { Idle, Armed, Paused };
    std::atomic<EState> State_ = EState::Idle;
};

void RunConcurrentQueries(SQLHDBC first, SQLHDBC second) {
    TStatementHandle a(first), b(second);
    std::barrier start(2);
    const auto work = [&](SQLHSTMT stmt) {
        start.arrive_and_wait();
        for (int i = 0; i < 30; ++i) {
            EXPECT_EQ(Execute(stmt), SQL_SUCCESS) << SqlState(stmt);
            EXPECT_EQ(SQLFetch(stmt), SQL_SUCCESS);
            SQLINTEGER value = 0;
            EXPECT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &value, sizeof(value), nullptr), SQL_SUCCESS);
            EXPECT_EQ(value, 42);
        }
    };
    std::jthread worker(work, a.Handle);
    work(b.Handle);
}

TEST(Concurrency, IndependentConnections) {
    TConnectionHandles a, b;
    RunConcurrentQueries(a.Dbc, b.Dbc);
}

TEST(Concurrency, AutocommitStatementsDoNotUseTransactionLock) {
    TConnectionHandles connection;
    auto owner = CheckedHandle<TConnection>(connection.Dbc);
    auto transaction = owner->LockTransaction();
    auto queries = std::async(std::launch::async, [&] {
        RunConcurrentQueries(connection.Dbc, connection.Dbc);
    });
    EXPECT_EQ(queries.wait_for(2s), std::future_status::ready);
    transaction.unlock();
    queries.get();
}

TEST(Concurrency, ParameterArraysReleaseCompletedSessions) {
    TConnectionHandles connection;
    auto owner = CheckedHandle<TConnection>(connection.Dbc);
    const auto count = owner->GetClient()->GetActiveSessionsLimit();
    TStatementHandle setup(connection.Dbc);
    ASSERT_EQ(Execute(setup.Handle, "DROP TABLE IF EXISTS odbc_concurrency_params"), SQL_SUCCESS);
    ASSERT_EQ(Execute(setup.Handle,
        "CREATE TABLE odbc_concurrency_params (id Int32, PRIMARY KEY(id))"), SQL_SUCCESS);
    std::vector<std::unique_ptr<TStatementHandle>> statements;
    std::vector<std::array<SQLINTEGER, 2>> parameters(count, {1, 2});
    for (int i = 0; i < count; ++i) {
        auto stmt = std::make_unique<TStatementHandle>(connection.Dbc);
        parameters[i] = {2 * i, 2 * i + 1};
        ASSERT_EQ(SQLPrepare(stmt->Handle, reinterpret_cast<SQLCHAR*>(const_cast<char*>(
            "UPSERT INTO odbc_concurrency_params (id) VALUES (?)")),
                             SQL_NTS), SQL_SUCCESS);
        ASSERT_EQ(SQLSetStmtAttr(stmt->Handle, SQL_ATTR_PARAMSET_SIZE,
                                reinterpret_cast<SQLPOINTER>(2), 0), SQL_SUCCESS);
        ASSERT_EQ(SQLBindParameter(stmt->Handle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                                  0, 0, parameters[i].data(), sizeof(SQLINTEGER), nullptr), SQL_SUCCESS);
        statements.push_back(std::move(stmt));
    }
    std::barrier start(count);
    {
        std::vector<std::jthread> workers;
        for (const auto& stmt : statements) {
            workers.emplace_back([&, handle = stmt->Handle] {
                start.arrive_and_wait();
                EXPECT_EQ(SQLExecute(handle), SQL_SUCCESS) << SqlState(handle);
            });
        }
    }
    EXPECT_TRUE(WaitUntil([&] { return owner->GetClient()->GetActiveSessionCount() == 0; }));
    EXPECT_EQ(Execute(setup.Handle, "DROP TABLE odbc_concurrency_params"), SQL_SUCCESS);
}

TEST(Concurrency, ExplicitTransactionStatements) {
    TConnectionHandles connection;
    TStatementHandle writer(connection.Dbc), reader(connection.Dbc);
    ASSERT_EQ(Execute(writer.Handle, "DROP TABLE IF EXISTS odbc_concurrency_tx"), SQL_SUCCESS);
    ASSERT_EQ(Execute(writer.Handle,
        "CREATE TABLE odbc_concurrency_tx (id Int32, PRIMARY KEY(id))"), SQL_SUCCESS);
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    RunConcurrentQueries(connection.Dbc, connection.Dbc);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_COMMIT), SQL_SUCCESS);
    RunConcurrentQueries(connection.Dbc, connection.Dbc);
    ASSERT_EQ(Execute(writer.Handle, "UPSERT INTO odbc_concurrency_tx (id) VALUES (42)"), SQL_SUCCESS);
    auto read = std::async(std::launch::async, [&] {
        return Execute(reader.Handle, "SELECT id FROM odbc_concurrency_tx WHERE id = 42");
    });
    ASSERT_EQ(read.get(), SQL_SUCCESS);
    EXPECT_EQ(SQLFetch(reader.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_ENV, connection.Env, SQL_ROLLBACK), SQL_SUCCESS);
    ASSERT_EQ(Execute(reader.Handle, "SELECT id FROM odbc_concurrency_tx WHERE id = 42"), SQL_SUCCESS);
    EXPECT_EQ(SQLFetch(reader.Handle), SQL_NO_DATA);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_COMMIT), SQL_SUCCESS);
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON), 0), SQL_SUCCESS);
    auto owner = CheckedHandle<TConnection>(connection.Dbc);
    EXPECT_TRUE(WaitUntil([&] { return owner->GetClient()->GetActiveSessionCount() == 0; }));
    EXPECT_EQ(Execute(writer.Handle, "DROP TABLE odbc_concurrency_tx"), SQL_SUCCESS);
}

TEST(Concurrency, AutocommitProgressAndActiveCancel) {
    TConnectionHandles connection;
    auto owned = std::make_unique<TPausedStatement>(CheckedHandle<TConnection>(connection.Dbc));
    auto* paused = owned.get();
    TStatementHandle slow(std::move(owned)), fast(connection.Dbc);
    ASSERT_EQ(SQLPrepare(slow.Handle,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT 42 AS value")), SQL_NTS), SQL_SUCCESS);
    SQLSMALLINT count = 0;
    ASSERT_EQ(SQLNumResultCols(slow.Handle, &count), SQL_SUCCESS);
    ASSERT_EQ(count, 1);
    SQLHDESC descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(slow.Handle, SQL_ATTR_IMP_ROW_DESC,
                            &descriptor, sizeof(descriptor), nullptr), SQL_SUCCESS);
    paused->PauseNextExecution();
    auto execution = std::async(std::launch::async, [&] { return SQLExecute(slow.Handle); });
    Y_DEFER { paused->Release(); };
    ASSERT_TRUE(paused->WaitUntilPaused());
    EXPECT_EQ(SQLSetDescField(descriptor, 0, SQL_DESC_COUNT, nullptr, 0), SQL_SUCCESS);
    EXPECT_EQ(Execute(fast.Handle), SQL_SUCCESS);
    {
        TStatementHandle idle(connection.Dbc);
    }
    EXPECT_EQ(execution.wait_for(0s), std::future_status::timeout);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_COMMIT), SQL_SUCCESS);
    CancelActiveExecution(slow.Handle, execution);
    paused->Release();
    EXPECT_EQ(execution.wait_for(5s), std::future_status::ready);
    EXPECT_EQ(execution.get(), SQL_SUCCESS) << SqlState(slow.Handle);
    EXPECT_EQ(SQLFetch(slow.Handle), SQL_NO_DATA);
    EXPECT_EQ(SQLGetDescField(descriptor, 0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(SQLFetch(fast.Handle), SQL_SUCCESS);
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(SQLCancel(slow.Handle), SQL_SUCCESS);
        EXPECT_EQ(Execute(fast.Handle), SQL_SUCCESS);
        EXPECT_EQ(Execute(slow.Handle), SQL_SUCCESS);
    }
}

TEST(Concurrency, CancelWhileSnapshottingParameters) {
    TConnectionHandles connection;
    for (bool atExec : {false, true}) {
        SCOPED_TRACE(atExec ? "data at execution" : "query execution");
        TStatementHandle statement(connection.Dbc);
        ASSERT_EQ(SQLPrepare(statement.Handle,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(atExec ? "SELECT ?" : "SELECT 42 AS value")), SQL_NTS), SQL_SUCCESS);
        SQLINTEGER token = 0;
        SQLLEN indicator = SQL_DATA_AT_EXEC;
        if (atExec) {
            ASSERT_EQ(SQLBindParameter(statement.Handle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                0, 0, &token, sizeof(token), &indicator), SQL_SUCCESS);
        }
        CancelWhileWaitingForDescriptor(statement.Handle, SQL_ATTR_APP_PARAM_DESC, [&] {
            return SQLExecute(statement.Handle);
        });
        EXPECT_FALSE(CheckedHandle<TStatement>(statement.Handle)->NeedsData());
        EXPECT_EQ(SQLExecute(statement.Handle), atExec ? SQL_NEED_DATA : SQL_SUCCESS);
        EXPECT_EQ(SQLCancel(statement.Handle), SQL_SUCCESS);
    }
}

TEST(Concurrency, CancelWhileResettingMetadata) {
    TConnectionHandles connection;
    TStatementHandle setup(connection.Dbc), statement(connection.Dbc);
    ASSERT_EQ(Execute(setup.Handle, "DROP TABLE IF EXISTS odbc_concurrency_cancel_metadata"), SQL_SUCCESS);
    ASSERT_EQ(Execute(setup.Handle,
        "CREATE TABLE odbc_concurrency_cancel_metadata (id Int32, PRIMARY KEY(id))"), SQL_SUCCESS);
    for (auto metadata : {SQLTables, SQLColumns}) {
        SCOPED_TRACE(metadata == SQLTables ? "SQLTables" : "SQLColumns");
        CancelWhileWaitingForDescriptor(statement.Handle, SQL_ATTR_IMP_ROW_DESC, [&] {
            return metadata(statement.Handle, nullptr, 0, nullptr, 0,
                reinterpret_cast<SQLCHAR*>(const_cast<char*>("odbc_concurrency_cancel_metadata")),
                SQL_NTS, nullptr, 0);
        });
    }
    EXPECT_EQ(Execute(setup.Handle, "DROP TABLE odbc_concurrency_cancel_metadata"), SQL_SUCCESS);
}

TEST(Concurrency, LocalCancelPreservesTransactionSession) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc), other(connection.Dbc);
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    auto owner = CheckedHandle<TConnection>(connection.Dbc);
    const auto sessionId = owner->GetOrCreateQuerySession().GetId();
    const auto transactionId = owner->GetTx()->GetId();
    EXPECT_EQ(SQLCancel(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(owner->GetOrCreateQuerySession().GetId(), sessionId);
    ASSERT_TRUE(owner->GetTx().has_value());
    EXPECT_EQ(owner->GetTx()->GetId(), transactionId);
    EXPECT_EQ(Execute(other.Handle), SQL_SUCCESS);
    EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_COMMIT), SQL_SUCCESS);
}

TEST(Concurrency, ActiveLocalCancelPreservesTransactionSession) {
    TConnectionHandles connection;
    auto owner = CheckedHandle<TConnection>(connection.Dbc);
    auto owned = std::make_unique<TPausedStatement>(owner);
    auto* paused = owned.get();
    TStatementHandle statement(std::move(owned));
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    const auto sessionId = owner->GetOrCreateQuerySession().GetId();
    const auto transactionId = owner->GetTx()->GetId();
    paused->PauseNextExecution();
    auto execution = std::async(std::launch::async, [&] {
        return Execute(statement.Handle);
    });
    Y_DEFER { paused->Release(); };
    ASSERT_TRUE(paused->WaitUntilPaused());
    CancelActiveExecution(statement.Handle, execution);
    paused->Release();
    EXPECT_EQ(execution.wait_for(5s), std::future_status::ready);
    EXPECT_EQ(execution.get(), SQL_SUCCESS) << SqlState(statement.Handle);
    EXPECT_EQ(SQLFetch(statement.Handle), SQL_NO_DATA);
    EXPECT_EQ(owner->GetOrCreateQuerySession().GetId(), sessionId);
    ASSERT_TRUE(owner->GetTx().has_value());
    EXPECT_EQ(owner->GetTx()->GetId(), transactionId);
    EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_ROLLBACK), SQL_SUCCESS);
}

TEST(Concurrency, SameHandleAndCrossThreadHandoff) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    std::barrier start(2);
    const auto work = [&] {
        start.arrive_and_wait();
        for (int i = 0; i < 30; ++i) {
            EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
            SQLULEN value = 0;
            EXPECT_EQ(SQLGetStmtAttr(statement.Handle, SQL_ATTR_CURSOR_TYPE,
                                    &value, sizeof(value), nullptr), SQL_SUCCESS);
        }
    };
    {
        std::jthread worker(work);
        work();
    }
    EXPECT_EQ(SQLFetch(statement.Handle), SQL_SUCCESS);
    {
        std::jthread nextOwner([&] {
            SQLINTEGER value = 0;
            EXPECT_EQ(SQLGetData(statement.Handle, 1, SQL_C_LONG,
                                 &value, sizeof(value), nullptr), SQL_SUCCESS);
            EXPECT_EQ(value, 42);
        });
    }
    auto owner = CheckedHandle<TStatement>(statement.Handle);
    auto operation = owner->LockOperation();
    owner->AddError("HYT00", 0, "Execution timed out");
    auto cancel = std::async(std::launch::async, [&] { return SQLCancel(statement.Handle); });
    const auto ready = cancel.wait_for(1s);
    operation.unlock();
    EXPECT_EQ(ready, std::future_status::ready);
    EXPECT_EQ(cancel.get(), SQL_ERROR);
    EXPECT_EQ(SqlState(statement.Handle), "HYT00");
}

TEST(Concurrency, LocalCancelDoesNotRetainAutocommitSessions) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    auto owner = CheckedHandle<TConnection>(connection.Dbc);
    for (int i = 0; i < 60; ++i) {
        ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
        EXPECT_EQ(SQLCancel(statement.Handle), SQL_SUCCESS);
        EXPECT_TRUE(WaitUntil([&] { return owner->GetClient()->GetActiveSessionCount() == 0; }));
    }
}

TEST(Concurrency, ParentFreeWaitsForChildPublication) {
    SQLHENV env = SQL_NULL_HENV;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    auto* parent = CheckedHandle<TEnvironment>(env);
    auto connection = std::make_unique<TConnection>(parent);
    auto* child = connection.get();
    auto publication = parent->LockOperation();
    EXPECT_TRUE(parent->GetChildren().empty());
    auto freeing = std::async(std::launch::async, [&] { return SQLFreeHandle(SQL_HANDLE_ENV, env); });
    const auto ready = freeing.wait_for(50ms);
    parent->AddChild(child);
    SQLHDBC dbc = static_cast<THandle*>(connection.release());
    publication.unlock();
    EXPECT_EQ(ready, std::future_status::timeout);
    EXPECT_EQ(freeing.get(), SQL_ERROR);
    EXPECT_EQ(SqlState(env, SQL_HANDLE_ENV), "HY010");
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DBC, dbc), SQL_SUCCESS);
    {
        auto operation = parent->LockOperation();
        EXPECT_TRUE(parent->GetChildren().empty());
    }
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
}

TEST(Concurrency, CancelQueuedTransactionDoesNotWaitOrAbortOtherStatements) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    auto owner = CheckedHandle<TConnection>(connection.Dbc);
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    const auto sessionId = owner->GetOrCreateQuerySession().GetId();
    const auto transactionId = owner->GetTx()->GetId();
    auto stmt = CheckedHandle<TStatement>(statement.Handle);
    auto transaction = owner->LockTransaction();
    auto execution = std::async(std::launch::async, [&] { return Execute(statement.Handle); });
    const bool started = WaitUntil([&] { return stmt->IsExecuting(); });
    EXPECT_TRUE(started);
    auto cancel = std::async(std::launch::async, [&] { return SQLCancel(statement.Handle); });
    const auto ready = cancel.wait_for(1s);
    EXPECT_EQ(ready, std::future_status::ready);
    if (ready == std::future_status::ready) {
        EXPECT_TRUE(stmt->IsExecuting());
        EXPECT_TRUE(stmt->IsCancelRequested());
        EXPECT_EQ(SQLCancel(statement.Handle), SQL_SUCCESS);
    }
    transaction.unlock();
    EXPECT_EQ(cancel.get(), SQL_SUCCESS);
    EXPECT_EQ(execution.get(), SQL_ERROR);
    EXPECT_EQ(SqlState(statement.Handle), "HY008");
    EXPECT_EQ(owner->GetOrCreateQuerySession().GetId(), sessionId);
    ASSERT_TRUE(owner->GetTx().has_value());
    EXPECT_EQ(owner->GetTx()->GetId(), transactionId);
    TStatementHandle other(connection.Dbc);
    EXPECT_EQ(Execute(other.Handle), SQL_SUCCESS);
    EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_ROLLBACK), SQL_SUCCESS);
}

TEST(Concurrency, DescriptorCopyMutationAndFree) {
    TConnectionHandles connection, otherConnection;
    TStatementHandle a(connection.Dbc), b(connection.Dbc);
    SQLHDESC first = SQL_NULL_HDESC, second = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, connection.Dbc, &first), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, otherConnection.Dbc, &second), SQL_SUCCESS);
    ASSERT_EQ(SQLSetStmtAttr(a.Handle, SQL_ATTR_APP_ROW_DESC, first, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLSetStmtAttr(b.Handle, SQL_ATTR_APP_ROW_DESC, first, 0), SQL_SUCCESS);
    SQLINTEGER firstValue = 0, secondValue = 0;
    ASSERT_EQ(SQLBindCol(a.Handle, 1, SQL_C_LONG, &firstValue, sizeof(firstValue), nullptr), SQL_SUCCESS);
    ASSERT_EQ(SQLCopyDesc(first, first), SQL_SUCCESS);
    SQLSMALLINT count = 0;
    ASSERT_EQ(SQLGetDescField(first, 0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(count, 1);
    SQLPOINTER data = nullptr;
    ASSERT_EQ(SQLGetDescField(first, 1, SQL_DESC_DATA_PTR, &data, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(data, &firstValue);
    ASSERT_EQ(SQLCopyDesc(first, second), SQL_SUCCESS);
    ASSERT_EQ(SQLSetDescField(second, 1, SQL_DESC_DATA_PTR, &secondValue, 0), SQL_SUCCESS);
    ASSERT_EQ(Execute(a.Handle), SQL_SUCCESS);
    ASSERT_EQ(Execute(b.Handle), SQL_SUCCESS);
    std::barrier start(5);
    {
        auto attributes = [&](SQLHDBC dbc) {
            start.arrive_and_wait();
            for (int i = 0; i < 100; ++i) {
                EXPECT_EQ(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT,
                    reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON), 0), SQL_SUCCESS);
            }
        };
        std::jthread firstAttributes(attributes, connection.Dbc);
        std::jthread secondAttributes(attributes, otherConnection.Dbc);
        const auto copy = [&](SQLHDESC source, SQLHDESC target) {
            start.arrive_and_wait();
            for (int i = 0; i < 100; ++i) {
                EXPECT_EQ(SQLCopyDesc(source, target), SQL_SUCCESS);
            }
        };
        std::jthread forward(copy, first, second);
        std::jthread reverse(copy, second, first);
        start.arrive_and_wait();
        EXPECT_EQ(SQLFetch(a.Handle), SQL_SUCCESS);
    }
    EXPECT_TRUE(firstValue == 42 || secondValue == 42);
    firstValue = secondValue = 0;
    EXPECT_EQ(SQLFetch(b.Handle), SQL_SUCCESS);
    EXPECT_TRUE(firstValue == 42 || secondValue == 42);
    ASSERT_EQ(Execute(a.Handle), SQL_SUCCESS);
    std::barrier freeing(2);
    auto fetch = std::async(std::launch::async, [&] {
        freeing.arrive_and_wait();
        return SQLFetch(a.Handle);
    });
    freeing.arrive_and_wait();
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, first), SQL_SUCCESS);
    EXPECT_EQ(fetch.get(), SQL_SUCCESS);
    SQLHDESC restored = SQL_NULL_HDESC;
    EXPECT_EQ(SQLGetStmtAttr(a.Handle, SQL_ATTR_APP_ROW_DESC,
                            &restored, sizeof(restored), nullptr), SQL_SUCCESS);
    EXPECT_NE(restored, first);
    EXPECT_EQ(SQLGetStmtAttr(b.Handle, SQL_ATTR_APP_ROW_DESC,
                            &restored, sizeof(restored), nullptr), SQL_SUCCESS);
    EXPECT_NE(restored, first);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, second), SQL_SUCCESS);
}

TEST(HandleApi, NullAndWrongLiveHandleTypes) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    SQLHDESC descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, connection.Dbc, &descriptor), SQL_SUCCESS);
    for (const auto type : {SQL_HANDLE_STMT, SQL_HANDLE_DESC}) {
        EXPECT_EQ(SQLAllocHandle(type, connection.Dbc, nullptr), SQL_ERROR);
        EXPECT_EQ(SqlState(connection.Dbc, SQL_HANDLE_DBC), "HY009");
    }
    SQLULEN value = 0;
    for (SQLHANDLE handle : std::array<SQLHANDLE, 4>{SQL_NULL_HANDLE, connection.Dbc, statement.Handle, descriptor}) {
        EXPECT_EQ(SQLGetEnvAttr(handle, SQL_ATTR_ODBC_VERSION, &value, sizeof(value), nullptr), SQL_INVALID_HANDLE);
    }
    for (SQLHANDLE handle : std::array<SQLHANDLE, 4>{SQL_NULL_HANDLE, connection.Env, statement.Handle, descriptor}) {
        EXPECT_EQ(SQLGetConnectAttr(handle, SQL_ATTR_AUTOCOMMIT, &value, sizeof(value), nullptr), SQL_INVALID_HANDLE);
    }
    for (SQLHANDLE handle : std::array<SQLHANDLE, 4>{SQL_NULL_HANDLE, connection.Env, connection.Dbc, descriptor}) {
        EXPECT_EQ(SQLGetStmtAttr(handle, SQL_ATTR_CURSOR_TYPE, &value, sizeof(value), nullptr), SQL_INVALID_HANDLE);
        EXPECT_EQ(SQLCancel(handle), SQL_INVALID_HANDLE);
    }
    for (SQLHANDLE handle : std::array<SQLHANDLE, 4>{SQL_NULL_HANDLE, connection.Env, connection.Dbc, statement.Handle}) {
        EXPECT_EQ(SQLGetDescField(handle, 0, SQL_DESC_COUNT, &value, 0, nullptr), SQL_INVALID_HANDLE);
        EXPECT_EQ(SQLCopyDesc(handle, descriptor), SQL_INVALID_HANDLE);
        EXPECT_EQ(SQLCopyDesc(descriptor, handle), SQL_INVALID_HANDLE);
    }
    const std::array<SQLHANDLE, 4> handles = {connection.Env, connection.Dbc, statement.Handle, descriptor};
    const std::array<SQLSMALLINT, 4> types = {SQL_HANDLE_ENV, SQL_HANDLE_DBC, SQL_HANDLE_STMT, SQL_HANDLE_DESC};
    for (size_t i = 0; i < handles.size(); ++i) {
        EXPECT_EQ(SQLFreeHandle(types[i], SQL_NULL_HANDLE), SQL_INVALID_HANDLE);
        EXPECT_EQ(SQLFreeHandle(999, handles[i]), SQL_INVALID_HANDLE);
        for (size_t j = 0; j < handles.size(); ++j) {
            if (i != j) {
                EXPECT_EQ(SQLFreeHandle(types[i], handles[j]), SQL_INVALID_HANDLE);
            }
        }
    }
    EXPECT_EQ(SQLFreeHandle(999, SQL_NULL_HANDLE), SQL_INVALID_HANDLE);
    EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, descriptor), SQL_SUCCESS);
}

TEST(HandleApi, AllocationValidatesParentAndOutput) {
    SQLHENV env = SQL_NULL_HENV;
    SQLHDBC dbc = SQL_NULL_HDBC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                          reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    for (const auto type : {SQL_HANDLE_STMT, SQL_HANDLE_DESC}) {
        SQLHANDLE output = env;
        EXPECT_EQ(SQLAllocHandle(type, dbc, &output), SQL_ERROR);
        EXPECT_EQ(output, nullptr);
        EXPECT_EQ(SqlState(dbc, SQL_HANDLE_DBC), "08003");
        EXPECT_EQ(SQLAllocHandle(type, dbc, nullptr), SQL_ERROR);
        EXPECT_EQ(SqlState(dbc, SQL_HANDLE_DBC), "HY009");
    }
    for (const auto type : {SQL_HANDLE_DBC, SQL_HANDLE_STMT, SQL_HANDLE_DESC}) {
        SQLHANDLE output = env;
        EXPECT_EQ(SQLAllocHandle(type, SQL_NULL_HANDLE, &output), SQL_INVALID_HANDLE);
        EXPECT_EQ(output, nullptr);
    }
    SQLHANDLE output = env;
    EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, dbc, &output), SQL_INVALID_HANDLE);
    EXPECT_EQ(output, nullptr);
    EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, nullptr), SQL_ERROR);
    EXPECT_EQ(SqlState(env, SQL_HANDLE_ENV), "HY009");
    EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, nullptr), SQL_ERROR);
    output = env;
    EXPECT_EQ(SQLAllocHandle(999, env, &output), SQL_ERROR);
    EXPECT_EQ(output, nullptr);
    EXPECT_EQ(SQLDisconnect(dbc), SQL_ERROR);
    EXPECT_EQ(SqlState(dbc, SQL_HANDLE_DBC), "08003");
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DBC, dbc), SQL_SUCCESS);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
}

TEST(HandleApi, ConnectedConnectionCannotBeFreed) {
    TConnectionHandles connection;
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DBC, connection.Dbc), SQL_ERROR);
    EXPECT_EQ(SqlState(connection.Dbc, SQL_HANDLE_DBC), "HY010");
    TStatementHandle statement(connection.Dbc);
    EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
}

TEST(HandleApi, ImplicitDescriptorCannotBeFreed) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    for (const auto attribute : {SQL_ATTR_APP_ROW_DESC, SQL_ATTR_APP_PARAM_DESC,
                                 SQL_ATTR_IMP_ROW_DESC, SQL_ATTR_IMP_PARAM_DESC}) {
        SQLHDESC descriptor = SQL_NULL_HDESC;
        ASSERT_EQ(SQLGetStmtAttr(statement.Handle, attribute, &descriptor, sizeof(descriptor), nullptr), SQL_SUCCESS);
        EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, descriptor), SQL_ERROR);
        EXPECT_EQ(SqlState(descriptor, SQL_HANDLE_DESC), "HY017");
        SQLSMALLINT count = -1;
        EXPECT_EQ(SQLGetDescField(descriptor, 0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_SUCCESS);
        EXPECT_EQ(count, 0);
    }
    EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
}

TEST(HandleApi, DescriptorAssociationRestoresOriginalAndRejectsForeignHandles) {
    TConnectionHandles connection, otherConnection;
    TStatementHandle statement(connection.Dbc), other(connection.Dbc);
    SQLHDESC explicitDescriptor = SQL_NULL_HDESC, foreignExplicit = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, connection.Dbc, &explicitDescriptor), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, otherConnection.Dbc, &foreignExplicit), SQL_SUCCESS);
    for (const auto attribute : {SQL_ATTR_APP_ROW_DESC, SQL_ATTR_APP_PARAM_DESC}) {
        SQLHDESC original = SQL_NULL_HDESC, foreignImplicit = SQL_NULL_HDESC, current = SQL_NULL_HDESC;
        ASSERT_EQ(SQLGetStmtAttr(statement.Handle, attribute, &original, sizeof(original), nullptr), SQL_SUCCESS);
        ASSERT_EQ(SQLGetStmtAttr(other.Handle, attribute, &foreignImplicit, sizeof(foreignImplicit), nullptr), SQL_SUCCESS);
        ASSERT_EQ(SQLSetStmtAttr(statement.Handle, attribute, explicitDescriptor, 0), SQL_SUCCESS);
        EXPECT_EQ(SQLSetStmtAttr(statement.Handle, attribute, original, 0), SQL_SUCCESS);
        EXPECT_EQ(SQLGetStmtAttr(statement.Handle, attribute, &current, sizeof(current), nullptr), SQL_SUCCESS);
        EXPECT_EQ(current, original);
        ASSERT_EQ(SQLSetStmtAttr(statement.Handle, attribute, explicitDescriptor, 0), SQL_SUCCESS);
        EXPECT_EQ(SQLSetStmtAttr(statement.Handle, attribute, SQL_NULL_HDESC, 0), SQL_SUCCESS);
        EXPECT_EQ(SQLGetStmtAttr(statement.Handle, attribute, &current, sizeof(current), nullptr), SQL_SUCCESS);
        EXPECT_EQ(current, original);
        EXPECT_EQ(SQLSetStmtAttr(statement.Handle, attribute, foreignImplicit, 0), SQL_ERROR);
        EXPECT_EQ(SqlState(statement.Handle), "HY017");
        EXPECT_EQ(SQLSetStmtAttr(statement.Handle, attribute, foreignExplicit, 0), SQL_ERROR);
        EXPECT_EQ(SqlState(statement.Handle), "HY024");
        EXPECT_EQ(SQLGetStmtAttr(statement.Handle, attribute, &current, sizeof(current), nullptr), SQL_SUCCESS);
        EXPECT_EQ(current, original);
    }
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, explicitDescriptor), SQL_SUCCESS);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, foreignExplicit), SQL_SUCCESS);
}

TEST(HandleApi, FreeBoundDescriptorsRestoresOriginalBindingsAndMetadata) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    SQLHDESC originalArd = SQL_NULL_HDESC, originalApd = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(statement.Handle, SQL_ATTR_APP_ROW_DESC, &originalArd, sizeof(originalArd), nullptr), SQL_SUCCESS);
    ASSERT_EQ(SQLGetStmtAttr(statement.Handle, SQL_ATTR_APP_PARAM_DESC, &originalApd, sizeof(originalApd), nullptr), SQL_SUCCESS);
    SQLINTEGER originalValue = 0, originalParameter = 17, explicitValue = 0, explicitParameter = 29;
    ASSERT_EQ(SQLBindCol(statement.Handle, 1, SQL_C_LONG, &originalValue, sizeof(originalValue), nullptr), SQL_SUCCESS);
    ASSERT_EQ(SQLBindParameter(statement.Handle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                              0, 0, &originalParameter, sizeof(originalParameter), nullptr), SQL_SUCCESS);
    SQLHDESC ard = SQL_NULL_HDESC, apd = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, connection.Dbc, &ard), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, connection.Dbc, &apd), SQL_SUCCESS);
    ASSERT_EQ(SQLSetStmtAttr(statement.Handle, SQL_ATTR_APP_ROW_DESC, ard, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLSetStmtAttr(statement.Handle, SQL_ATTR_APP_PARAM_DESC, apd, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLBindCol(statement.Handle, 1, SQL_C_LONG, &explicitValue, sizeof(explicitValue), nullptr), SQL_SUCCESS);
    ASSERT_EQ(SQLBindParameter(statement.Handle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                              0, 0, &explicitParameter, sizeof(explicitParameter), nullptr), SQL_SUCCESS);
    ASSERT_EQ(SQLPrepare(statement.Handle, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ? AS value")), SQL_NTS), SQL_SUCCESS);
    ASSERT_EQ(SQLExecute(statement.Handle), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(explicitValue, explicitParameter);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, ard), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, apd), SQL_SUCCESS);
    for (const auto [attribute, original] : {std::pair{SQL_ATTR_APP_ROW_DESC, originalArd},
                                           std::pair{SQL_ATTR_APP_PARAM_DESC, originalApd}}) {
        SQLHDESC restored = SQL_NULL_HDESC;
        ASSERT_EQ(SQLGetStmtAttr(statement.Handle, attribute, &restored, sizeof(restored), nullptr), SQL_SUCCESS);
        EXPECT_EQ(restored, original);
        SQLSMALLINT count = 0;
        EXPECT_EQ(SQLGetDescField(restored, 0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_SUCCESS);
        EXPECT_EQ(count, 1);
    }
    SQLSMALLINT count = 0;
    EXPECT_EQ(SQLNumParams(statement.Handle, &count), SQL_SUCCESS);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(SQLNumResultCols(statement.Handle, &count), SQL_SUCCESS);
    EXPECT_EQ(count, 1);
    ASSERT_EQ(SQLExecute(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLFetch(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(originalValue, originalParameter);
}

TEST(HandleApi, StatementFreePreservesNeedDataUntilCancel) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    ASSERT_EQ(SQLPrepare(statement.Handle, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ?")), SQL_NTS), SQL_SUCCESS);
    SQLLEN indicator = SQL_DATA_AT_EXEC;
    SQLINTEGER token = 0;
    ASSERT_EQ(SQLBindParameter(statement.Handle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                              0, 0, &token, sizeof(token), &indicator), SQL_SUCCESS);
    SQLHDESC original = SQL_NULL_HDESC, descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(statement.Handle, SQL_ATTR_APP_PARAM_DESC,
                             &original, sizeof(original), nullptr), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, connection.Dbc, &descriptor), SQL_SUCCESS);
    ASSERT_EQ(SQLCopyDesc(original, descriptor), SQL_SUCCESS);
    ASSERT_EQ(SQLSetStmtAttr(statement.Handle, SQL_ATTR_APP_PARAM_DESC, descriptor, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLExecute(statement.Handle), SQL_NEED_DATA);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_STMT, statement.Handle), SQL_ERROR);
    EXPECT_EQ(SqlState(statement.Handle), "HY010");
    EXPECT_EQ(SQLSetStmtAttr(statement.Handle, SQL_ATTR_APP_PARAM_DESC, nullptr, 0), SQL_ERROR);
    EXPECT_EQ(SqlState(statement.Handle), "HY010");
    EXPECT_EQ(SQLSetStmtAttr(statement.Handle, SQL_ATTR_QUERY_TIMEOUT, reinterpret_cast<SQLPOINTER>(1), 0), SQL_ERROR);
    EXPECT_EQ(SqlState(statement.Handle), "HY010");
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, descriptor), SQL_SUCCESS);
    SQLPOINTER requested = nullptr;
    EXPECT_EQ(SQLParamData(statement.Handle, &requested), SQL_NEED_DATA);
    EXPECT_EQ(requested, &token);
    ASSERT_EQ(SQLCancel(statement.Handle), SQL_SUCCESS);
    SQLHDESC restored = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(statement.Handle, SQL_ATTR_APP_PARAM_DESC,
                            &restored, sizeof(restored), nullptr), SQL_SUCCESS);
    EXPECT_EQ(restored, original);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_STMT, statement.Handle), SQL_SUCCESS);
    statement.Handle = SQL_NULL_HSTMT;
}

TEST(HandleApi, DisconnectFailurePreservesChildrenAndTransaction) {
    TConnectionHandles connection;
    TStatementHandle transaction(connection.Dbc), data(connection.Dbc);
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    ASSERT_EQ(Execute(transaction.Handle), SQL_SUCCESS);
    ASSERT_EQ(SQLPrepare(data.Handle, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ?")), SQL_NTS), SQL_SUCCESS);
    SQLINTEGER token = 0;
    SQLLEN indicator = SQL_DATA_AT_EXEC;
    ASSERT_EQ(SQLBindParameter(data.Handle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                              0, 0, &token, sizeof(token), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLExecute(data.Handle), SQL_NEED_DATA);
    EXPECT_EQ(SQLDisconnect(connection.Dbc), SQL_ERROR);
    EXPECT_EQ(SqlState(connection.Dbc, SQL_HANDLE_DBC), "HY010");
    SQLPOINTER requested = nullptr;
    EXPECT_EQ(SQLParamData(data.Handle, &requested), SQL_NEED_DATA);
    EXPECT_EQ(requested, &token);
    EXPECT_EQ(SQLFetch(transaction.Handle), SQL_SUCCESS);
    ASSERT_EQ(SQLCancel(data.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLDisconnect(connection.Dbc), SQL_ERROR);
    EXPECT_EQ(SqlState(connection.Dbc, SQL_HANDLE_DBC), "25000");
    EXPECT_EQ(Execute(transaction.Handle), SQL_SUCCESS);
    ASSERT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_ROLLBACK), SQL_SUCCESS);
    const auto disconnected = SQLDisconnect(connection.Dbc);
    if (disconnected == SQL_SUCCESS) {
        transaction.Handle = data.Handle = SQL_NULL_HSTMT;
    }
    EXPECT_EQ(disconnected, SQL_SUCCESS);
}

class TCountedStatement : public TStatement {
public:
    TCountedStatement(TConnection* connection, int& destroyed)
        : TStatement(connection), Destroyed_(destroyed) {}
    ~TCountedStatement() override { ++Destroyed_; }
private:
    int& Destroyed_;
};

class TCountedDescriptor : public TDescriptor {
public:
    TCountedDescriptor(TConnection* connection, int& destroyed)
        : TDescriptor(connection), Destroyed_(destroyed) {}
    ~TCountedDescriptor() override { ++Destroyed_; }
private:
    int& Destroyed_;
};

TEST(Concurrency, DisconnectDestroysChildrenAndReconnects) {
    int statementsDestroyed = 0, descriptorsDestroyed = 0;
    TConnectionHandles connection;
    auto* owner = CheckedHandle<TConnection>(connection.Dbc);
    TStatementHandle statement(std::make_unique<TCountedStatement>(owner, statementsDestroyed));
    auto descriptor = std::make_unique<TCountedDescriptor>(owner, descriptorsDestroyed);
    {
        auto lifecycle = owner->LockShared();
        auto operation = owner->LockOperation();
        owner->AddChild(descriptor.get());
        descriptor.release();
        EXPECT_EQ(owner->GetChildren().size(), 2u);
    }
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    const auto disconnected = SQLDisconnect(connection.Dbc);
    if (disconnected == SQL_SUCCESS) {
        statement.Handle = SQL_NULL_HSTMT;
    }
    ASSERT_EQ(disconnected, SQL_SUCCESS);
    EXPECT_EQ(statementsDestroyed, 1);
    EXPECT_EQ(descriptorsDestroyed, 1);
    {
        auto lifecycle = owner->LockShared();
        auto operation = owner->LockOperation();
        EXPECT_TRUE(owner->GetChildren().empty());
        EXPECT_FALSE(owner->GetClient());
        EXPECT_FALSE(owner->GetTx());
    }
    EXPECT_EQ(SQLDisconnect(connection.Dbc), SQL_ERROR);
    EXPECT_EQ(SqlState(connection.Dbc, SQL_HANDLE_DBC), "08003");
    ASSERT_EQ(SQLDriverConnect(connection.Dbc, nullptr,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>("Server=localhost:2136;Database=/local;")),
        SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT), SQL_SUCCESS);
    TStatementHandle reconnected(connection.Dbc);
    EXPECT_EQ(Execute(reconnected.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLFetch(reconnected.Handle), SQL_SUCCESS);
}

TEST(Concurrency, ParentCannotBeFreedWithLiveChildren) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, connection.Env), SQL_ERROR);
    EXPECT_EQ(SqlState(connection.Env, SQL_HANDLE_ENV), "HY010");
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DBC, connection.Dbc), SQL_ERROR);
    EXPECT_EQ(SqlState(connection.Dbc, SQL_HANDLE_DBC), "HY010");
    EXPECT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(SQLFetch(statement.Handle), SQL_SUCCESS);
}

} // namespace
