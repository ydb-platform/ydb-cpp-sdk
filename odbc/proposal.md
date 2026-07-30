# ODBC driver bindings

## Goal

Validate the YDB ODBC driver through established ODBC bindings. Applications use the binding's public API and select YDB through a DSN or connection string. Binding implementation code is not patched.

Languages with a maintained native YDB SDK are out of scope. PHP remains in scope because its native SDK is planned for deprecation.

## Binding matrix

Each language has one binding, one pinned upstream revision, its upstream database tests, and one runnable example.

| Tier | Language | Binding | Tests |
|---|---|---|---|
| Core | Erlang | [OTP `odbc`](https://github.com/erlang/otp/tree/master/lib/odbc) | `lib/odbc/test` Common Test cases |
| Core | PHP | [PDO_ODBC](https://github.com/php/php-src/tree/master/ext/pdo_odbc) | PDO_ODBC and generic PDO PHPT tests |
| Core | Haskell | [HDBC-odbc](https://github.com/hdbc/HDBC-odbc) | HDBC/HUnit database tests |
| Core | Ruby | [ruby-odbc](https://github.com/larskanis/ruby-odbc) | Upstream test scripts |
| Core | Lua | [LuaSQL ODBC](https://github.com/lunarmodules/luasql) | Common LuaSQL and ODBC parameter tests |
| Core | Perl | [DBD::ODBC](https://github.com/perl5-dbi/DBD-ODBC) | Upstream TAP tests |
| Core | R | [`odbc`](https://github.com/r-dbi/odbc) | `testthat` and DBItest |
| Core | Julia | [ODBC.jl](https://github.com/JuliaDatabases/ODBC.jl) | ODBC.jl, DBInterface, and Tables tests |
| Core | Tcl | [`tdbc::odbc`](https://core.tcl-lang.org/tdbc) | ODBC backend `tcltest` suite |
| Expansion | Raku | [DBDish::ODBC](https://github.com/salortiz/DBDish-ODBC) | Upstream and DBIish tests |
| Expansion | Crystal | [crystal-odbc](https://github.com/naqvis/crystal-odbc) | `crystal spec` |
| Expansion | Dart | [`dart_odbc`](https://pub.dev/packages/dart_odbc) | `dart test` |
| Expansion | D | [`odbc`](https://github.com/singingbush/odbc) | Upstream unit and integration tests |
| Expansion | OCaml | [`ocaml-odbc`](https://opam.ocaml.org/packages/odbc/) | Upstream tests |
| Expansion | Common Lisp | [CLSQL ODBC](https://github.com/sharplispers/clsql) | ODBC ASDF tests |
| Expansion | COBOL | [GixSQL ODBC](https://github.com/mridoni/gixsql) | ODBC regression tests |
| Expansion | Pascal | [Free Pascal SQLDB ODBC](https://gitlab.com/freepascal.org/fpc/source/-/tree/main/packages/fcl-db) | SQLDB connector tests |
| Expansion | Smalltalk | [Pharo-ODBC](https://github.com/pharo-rdbms/Pharo-ODBC) | SUnit tests |
| Expansion | Fortran | [`odbc.f`](https://davidpfister.github.io/odbc.f/) | Upstream fpm tests |

## Required driver behavior

### Connections

An ODBC connection is one endpoint/database pair. YDB requires both values for routing ([connection parameters](https://ydb.tech/docs/en/concepts/connect)).

Each `SQLHDBC` owns:

- one SDK `TDriver` configured with its endpoint and database;
- its query, table, and scheme clients;
- its query session and active transaction;
- its current catalog, credentials, and diagnostics.

The current implementation follows this model: `SQLAllocHandle(SQL_HANDLE_DBC)` creates a `TConnection`, and `TConnection::TYdbState` owns the SDK driver and clients. `SQLHENV` only tracks connection handles. Connections in the same environment therefore keep endpoint, database, session, transaction, and catalog state separate.

`SQL_ATTR_CURRENT_CATALOG` uses a path below the connected database as `TablePathPrefix`. Setting it to another database path recreates only that connection's SDK state.

Required tests:

- two database paths on one endpoint;
- two endpoints;
- simultaneous queries;
- independent transactions and catalogs;
- failure and disconnect of one connection while the other remains usable;
- driver-manager pooling keyed by endpoint, database, credentials, and TLS settings.

### Cursors

YDB returns result sets, not server-side ODBC cursors. The driver owns cursor state for each statement.

The cursor states are `before first`, `on row or rowset`, `after last`, and `closed`. `SQLFetch` and `SQLFetchScroll(SQL_FETCH_NEXT)` advance a forward cursor. A static cursor stores a result snapshot and implements `FIRST`, `LAST`, `PRIOR`, `ABSOLUTE`, and `RELATIVE`.

The cursor also owns:

- typed YDB rows and column metadata;
- row-wise and column-wise bindings;
- row-array status and processed-row counters;
- a separate chunk offset for each `SQLGetData` column;
- bounded memory with a statement-local spill file for static cursors.

Re-execution replaces the cursor. Close, cancel, commit, rollback, and disconnect release it according to the advertised cursor behavior.

### Binding contract

The driver must support the operations used by the Core bindings:

- DSN and connection-string connection;
- prepare, bind, execute, and data-at-execution parameters;
- scalar and rowset fetch;
- `NULL`, integer, floating-point, decimal, text, binary, date, time, and timestamp conversion;
- column, table, key, index, type, and result metadata;
- autocommit and explicit transactions;
- diagnostics through SQLSTATE and native YDB issues;
- independent statements and connections;
- deterministic cleanup after errors.

## Current limitations

| Area | Limitation |
|---|---|
| SQL dialect | Statements are YQL. The driver rewrites ODBC escapes and `?` parameters; it is not a general ANSI SQL translator. |
| Authentication | Connection strings currently configure only endpoint, database, and DSN. User/password, token, service-account, metadata credentials, TLS certificates, and custom IAM settings are not wired into `TDriverConfig`. |
| Retry classification | The driver cannot infer whether arbitrary SQL is idempotent. Autocommit statement retries use `TRetryOperationSettings::Idempotent(false)`. This enables only retries safe for a non-idempotent operation and may return an error with an unknown execution outcome. See [YDB retry settings](https://ydb.tech/docs/en/recipes/ydb-sdk/retry) and [error handling](https://ydb.tech/docs/en/reference/ydb-sdk/error_handling). |
| Explicit transactions | An ODBC transaction is not retried by the driver. Conflicts, node failures, maintenance, and network failures can abort it, including at commit. The application must open a new transaction and replay the entire unit of work in a retry loop. Retrying only the failed statement is incorrect ([query execution](https://ydb.tech/docs/en/concepts/query_execution/), [transactions](https://ydb.tech/docs/en/concepts/transactions)). |
| Transaction isolation | Read-write connections support serializable and snapshot read-write modes. ODBC read-committed and read-uncommitted requests are rejected. |
| Transaction scope | `SQLEndTran(SQL_HANDLE_ENV, ...)` completes each connection independently. It is not an atomic transaction across databases or endpoints. |
| Connection concurrency | An explicit transaction uses one SDK session. YDB sessions execute one query at a time, so concurrent statements on the same transaction connection require application serialization ([YDB errors FAQ](https://ydb.tech/docs/en/faq/errors)). |
| Cursor support | The current implementation is forward-only. `SQLFetchScroll` accepts only `SQL_FETCH_NEXT`; static scrolling and spill are planned. |
| Result sets | Only the first result set is exposed. `SQLMoreResults` returns `SQL_NO_DATA`. |
| Result buffering | Query execution uses the non-streaming SDK result and keeps it for cursor fetches. Large results can consume memory proportional to the result size. |
| Row counts | `SQLRowCount` returns `-1`; affected-row counts are not extracted from YDB query statistics. |
| Prepare | `SQLPrepare` stores the query and counts client-side parameter markers. It does not create a persistent server-side prepared statement. |
| Batches | Parameter arrays are accepted only for data-modification statements and execute sequentially. Earlier parameter sets may already be committed when a later set fails. |
| Cancellation | Execution is synchronous. `SQLCancel` clears local cursor and parameter state but does not interrupt an in-flight SDK request. |
| Metadata namespace | YDB paths are exposed as catalogs. Schemas are empty. |
| DDL | Autocommit DDL uses `NoTx`. DDL executed while autocommit is off is sent through the active transaction and may be rejected by YDB. |
| Optional ODBC features | Multiple result sets, stored procedures, output parameters, positioned updates, bookmarks, asynchronous execution, and ODBC batch operations are not implemented. |
| Thread safety | Handle state is mutable and has no internal locking. Applications must serialize access to the same ODBC handle. |

## Test repository

```text
odbc/tests/frameworks/
  registry.yaml
  <language>/
    upstream.lock
    run-tests
    convert-results
    example/
odbc/tests/reporting/
```

`registry.yaml` records the language, tier, runtime image, upstream URL and revision, archive checksum, test command, result format, and example command. CI verifies the checksum before running the unchanged upstream binding.

Each example uses only the binding's public API. It accepts `YDB_ODBC_DSN` or `YDB_ODBC_CONNECTION_STRING`, creates isolated test data, executes bound statements, iterates a cursor, demonstrates commit and rollback, and cleans up.

Test output is converted to Allure without changing the upstream runner. Reports include the binding version, runtime version, driver commit, YDB version, endpoint/database mode, and upstream checksum. Missing tests, an empty suite, infrastructure failure, and unexpected skips fail the job.

## CI

The complete matrix runs only:

- after a merge into `odbc-driver-feature`;
- for a pushed tag.

It does not run for pull requests, direct non-merge pushes, schedules, or manual dispatches.

Each job starts a pinned YDB version, registers the driver in a job-local `odbcinst.ini`, verifies the upstream source, runs the upstream tests and example, and uploads native and Allure results.

## Delivery order

1. Add the registry, framework runner template, source verification, result conversion, and report aggregation.
2. Add static cursor emulation and cursor integration tests.
3. Add two-endpoint and two-database isolation tests.
4. Onboard Core bindings.
5. Onboard Expansion bindings.
6. Enable a zero-regression gate for stable bindings.

## Acceptance

- Every selected binding runs from a pinned, verified upstream source.
- Binding implementation code is unchanged.
- Every upstream database test is executed or reported as unsupported with its original test identifier.
- Every binding has a runnable example.
- Unit and integration tests pass.
- Endpoint/database isolation tests pass.
- Reports contain no missing or unexpected skipped tests.
