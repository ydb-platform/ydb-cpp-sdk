#include "connection.h"
#include "environment.h"
#include "statement.h"

#include <gtest/gtest.h>

#include <barrier>
#include <array>
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
        SQLDisconnect(Dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, Dbc);
        SQLFreeHandle(SQL_HANDLE_ENV, Env);
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

    ~TStatementHandle() {
        if (Handle) {
            SQLFreeHandle(SQL_HANDLE_STMT, Handle);
        }
    }
};

SQLRETURN Execute(SQLHSTMT stmt, const char* query = "SELECT 42 AS value") {
    return SQLExecDirect(stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>(query)), SQL_NTS);
}

class TLongQuery {
    TConnectionHandles Monitoring_;
    TStatementHandle Observer_{Monitoring_.Dbc};
    const std::string Table_ = std::string("odbc_concurrency_")
        + ::testing::UnitTest::GetInstance()->current_test_info()->name();
public:
    const std::string Query = "SELECT SUM(a.id + b.id + c.id) FROM " + Table_
        + " AS a CROSS JOIN " + Table_ + " AS b CROSS JOIN " + Table_ + " AS c";

    TLongQuery() {
        Execute(Observer_.Handle, ("DROP TABLE IF EXISTS " + Table_).c_str());
        EXPECT_EQ(Execute(Observer_.Handle,
            ("CREATE TABLE " + Table_ + " (id Uint32, PRIMARY KEY(id))").c_str()), SQL_SUCCESS);
        EXPECT_EQ(Execute(Observer_.Handle, ("UPSERT INTO " + Table_ + " (id) SELECT v FROM "
            "AS_TABLE(ListMap(ListFromRange(1u, 1001u), ($x)->(AsStruct($x AS v))))").c_str()), SQL_SUCCESS);
    }

    ~TLongQuery() { Execute(Observer_.Handle, ("DROP TABLE " + Table_).c_str()); }

    bool Running() {
        const std::string query = "SELECT COUNT(*) FROM `.sys/query_sessions` WHERE Query = '"
            + Query + "' AND State = 'EXECUTING'";
        if (Execute(Observer_.Handle, query.c_str()) != SQL_SUCCESS
            || SQLFetch(Observer_.Handle) != SQL_SUCCESS) {
            return false;
        }
        SQLINTEGER count = 0;
        return SQLGetData(Observer_.Handle, 1, SQL_C_LONG, &count, sizeof(count), nullptr) == SQL_SUCCESS
            && count > 0;
    }
};

std::string SqlState(SQLHANDLE handle, SQLSMALLINT type = SQL_HANDLE_STMT) {
    SQLCHAR state[6] = {};
    SQLGetDiagRec(type, handle, 1, state, nullptr, nullptr, 0, nullptr);
    return reinterpret_cast<char*>(state);
}

void CancelActiveExecution(SQLHSTMT stmt, std::future<SQLRETURN>& execution) {
    auto cancellation = std::async(std::launch::async, [&] {
        auto owner = std::dynamic_pointer_cast<TStatement>(PinHandle(stmt));
        EXPECT_TRUE(owner->IsExecuting());
        EXPECT_EQ(execution.wait_for(0s), std::future_status::timeout);
        const auto result = SQLCancel(stmt);
        return result;
    });
    EXPECT_EQ(cancellation.wait_for(1s), std::future_status::ready);
    EXPECT_EQ(cancellation.get(), SQL_SUCCESS);
}

void ExpectCanceledExecution(std::future<SQLRETURN>& execution, SQLHSTMT stmt) {
    EXPECT_EQ(execution.wait_for(13s), std::future_status::ready);
    const auto result = execution.get();
    if (result == SQL_ERROR) {
        const auto state = SqlState(stmt);
        EXPECT_TRUE(state == "HY008" || state == "HYT00") << state;
    } else {
        EXPECT_EQ(result, SQL_SUCCESS);
    }
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
    auto owner = std::dynamic_pointer_cast<TConnection>(PinHandle(connection.Dbc));
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
    auto owner = std::dynamic_pointer_cast<TConnection>(PinHandle(connection.Dbc));
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
    auto owner = std::dynamic_pointer_cast<TConnection>(PinHandle(connection.Dbc));
    EXPECT_TRUE(WaitUntil([&] { return owner->GetClient()->GetActiveSessionCount() == 0; }));
    EXPECT_EQ(Execute(writer.Handle, "DROP TABLE odbc_concurrency_tx"), SQL_SUCCESS);
}

TEST(Concurrency, AutocommitProgressAndActiveCancel) {
    TConnectionHandles connection;
    TStatementHandle slow(connection.Dbc), fast(connection.Dbc);
    TLongQuery query;
    ASSERT_EQ(SQLPrepare(slow.Handle,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(query.Query.c_str())), SQL_NTS), SQL_SUCCESS);
    SQLSMALLINT count = 0;
    ASSERT_EQ(SQLNumResultCols(slow.Handle, &count), SQL_SUCCESS);
    ASSERT_EQ(count, 1);
    SQLHDESC descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(slow.Handle, SQL_ATTR_IMP_ROW_DESC,
                            &descriptor, sizeof(descriptor), nullptr), SQL_SUCCESS);
    auto execution = std::async(std::launch::async, [&] { return SQLExecute(slow.Handle); });
    EXPECT_TRUE(WaitUntil([&] { return query.Running(); }));
    EXPECT_EQ(SQLSetDescField(descriptor, 0, SQL_DESC_COUNT, nullptr, 0), SQL_SUCCESS);
    EXPECT_EQ(Execute(fast.Handle), SQL_SUCCESS);
    EXPECT_EQ(execution.wait_for(0s), std::future_status::timeout);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, connection.Dbc, SQL_COMMIT), SQL_SUCCESS);
    CancelActiveExecution(slow.Handle, execution);
    ExpectCanceledExecution(execution, slow.Handle);
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

TEST(Concurrency, LocalCancelPreservesTransactionSession) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc), other(connection.Dbc);
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    auto owner = std::dynamic_pointer_cast<TConnection>(PinHandle(connection.Dbc));
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
    TStatementHandle statement(connection.Dbc);
    TLongQuery query;
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    auto owner = std::dynamic_pointer_cast<TConnection>(PinHandle(connection.Dbc));
    const auto sessionId = owner->GetOrCreateQuerySession().GetId();
    const auto transactionId = owner->GetTx()->GetId();
    auto execution = std::async(std::launch::async, [&] {
        return Execute(statement.Handle, query.Query.c_str());
    });
    EXPECT_TRUE(WaitUntil([&] { return query.Running(); }));
    CancelActiveExecution(statement.Handle, execution);
    ExpectCanceledExecution(execution, statement.Handle);
    EXPECT_EQ(SQLFetch(statement.Handle), SQL_NO_DATA);
    EXPECT_EQ(owner->GetOrCreateQuerySession().GetId(), sessionId);
    ASSERT_TRUE(owner->GetTx().has_value());
    EXPECT_EQ(owner->GetTx()->GetId(), transactionId);
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
    auto owner = std::dynamic_pointer_cast<TStatement>(PinHandle(statement.Handle));
    auto operation = owner->LockOperation();
    owner->AddError("HYT00", 0, "Execution timed out");
    auto cancel = std::async(std::launch::async, [&] { return SQLCancel(statement.Handle); });
    const auto ready = cancel.wait_for(1s);
    operation.unlock();
    EXPECT_EQ(ready, std::future_status::ready);
    EXPECT_EQ(cancel.get(), SQL_SUCCESS);
    EXPECT_EQ(SqlState(statement.Handle), "HYT00");
}

TEST(Concurrency, LocalCancelDoesNotRetainAutocommitSessions) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    auto owner = std::dynamic_pointer_cast<TConnection>(PinHandle(connection.Dbc));
    for (int i = 0; i < 60; ++i) {
        ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
        EXPECT_EQ(SQLCancel(statement.Handle), SQL_SUCCESS);
        EXPECT_TRUE(WaitUntil([&] { return owner->GetClient()->GetActiveSessionCount() == 0; }));
    }
}

TEST(Concurrency, ParentFreeWaitsForChildPublicationAndUnwind) {
    SQLHENV env = SQL_NULL_HENV;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    auto parent = std::dynamic_pointer_cast<TEnvironment>(PinHandle(env));
    {
        auto connection = std::make_shared<TConnection>(parent);
        EXPECT_FALSE(parent->HasChildren());
        std::future<SQLRETURN> freeing;
        auto publication = parent->LockOperation();
        freeing = std::async(std::launch::async, [&] { return SQLFreeHandle(SQL_HANDLE_ENV, env); });
        const auto ready = freeing.wait_for(50ms);
        RegisterHandle(connection.get(), connection);
        publication.unlock();
        EXPECT_EQ(ready, std::future_status::timeout);
        EXPECT_EQ(freeing.get(), SQL_ERROR);
        EXPECT_EQ(SqlState(env, SQL_HANDLE_ENV), "HY010");
        // Partial statement/alias publication must unwind without leaving registry children.
        auto statement = std::make_shared<TStatement>(connection);
        RegisterHandle(statement.get(), statement);
        statement->RegisterDescriptors(statement);
        SQLHDESC descriptor = SQL_NULL_HDESC;
        ASSERT_EQ(SQLGetStmtAttr(statement.get(), SQL_ATTR_IMP_ROW_DESC,
                                &descriptor, sizeof(descriptor), nullptr), SQL_SUCCESS);
        statement->UnregisterDescriptors();
        UnregisterHandle(statement.get());
        EXPECT_FALSE(PinHandle(descriptor));
        EXPECT_FALSE(connection->HasChildren());
        UnregisterHandle(connection.get());
    }
    EXPECT_FALSE(parent->HasChildren());
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
}

TEST(Concurrency, CancelQueuedTransactionDoesNotWaitOrAbortOtherStatements) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    ASSERT_EQ(SQLSetConnectAttr(connection.Dbc, SQL_ATTR_AUTOCOMMIT,
                               reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0), SQL_SUCCESS);
    auto owner = std::dynamic_pointer_cast<TConnection>(PinHandle(connection.Dbc));
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    const auto sessionId = owner->GetOrCreateQuerySession().GetId();
    const auto transactionId = owner->GetTx()->GetId();
    auto stmt = std::dynamic_pointer_cast<TStatement>(PinHandle(statement.Handle));
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
    EXPECT_EQ(SQLGetDescField(first, 0, SQL_DESC_COUNT, &restored, 0, nullptr), SQL_INVALID_HANDLE);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, second), SQL_SUCCESS);
}

TEST(Concurrency, DisconnectAndFreeRaces) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    std::barrier start(3);
    auto use = std::async(std::launch::async, [&] {
        start.arrive_and_wait();
        return Execute(statement.Handle);
    });
    auto disconnect = std::async(std::launch::async, [&] {
        start.arrive_and_wait();
        return SQLDisconnect(connection.Dbc);
    });
    start.arrive_and_wait();
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_STMT, statement.Handle), SQL_SUCCESS);
    EXPECT_EQ(disconnect.get(), SQL_SUCCESS);
    const auto result = use.get();
    EXPECT_TRUE(result == SQL_SUCCESS || result == SQL_ERROR || result == SQL_INVALID_HANDLE);
    EXPECT_EQ(SQLCancel(statement.Handle), SQL_INVALID_HANDLE);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_STMT, statement.Handle), SQL_INVALID_HANDLE);
    statement.Handle = SQL_NULL_HSTMT;
}

TEST(Concurrency, ImplicitDescriptorIsRetiredWithStatement) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    SQLHDESC descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(statement.Handle, SQL_ATTR_IMP_ROW_DESC,
                            &descriptor, sizeof(descriptor), nullptr), SQL_SUCCESS);
    auto pin = PinHandle(descriptor);
    ASSERT_TRUE(pin);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_STMT, statement.Handle), SQL_SUCCESS);
    statement.Handle = SQL_NULL_HSTMT;
    SQLSMALLINT count = 0;
    EXPECT_EQ(SQLGetDescField(descriptor, 0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_INVALID_HANDLE);
    EXPECT_TRUE(pin->IsRetired());
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DBC, connection.Dbc), SQL_SUCCESS);
    connection.Dbc = SQL_NULL_HDBC;
}

TEST(Concurrency, DisconnectClearsImplicitMetadataWithoutStatementCall) {
    TConnectionHandles connection;
    TStatementHandle statement(connection.Dbc);
    ASSERT_EQ(Execute(statement.Handle), SQL_SUCCESS);
    SQLHDESC descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLGetStmtAttr(statement.Handle, SQL_ATTR_IMP_ROW_DESC,
                            &descriptor, sizeof(descriptor), nullptr), SQL_SUCCESS);
    SQLSMALLINT count = 0;
    ASSERT_EQ(SQLGetDescField(descriptor, 0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(SQLDisconnect(connection.Dbc), SQL_SUCCESS);
    EXPECT_EQ(SQLGetDescField(descriptor, 0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(count, 0);
}

} // namespace
