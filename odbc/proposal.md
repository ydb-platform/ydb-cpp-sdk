# Odbc driver
Odbc is a database connection layer, which gives users the opportunity to execute sql and interact with a database using a standardised C ABI. This document regulates how should the odbc driver for YDB be implemented, which functionality it is supposed to cover and the acceptance criteria.

## Goal

The driver should provide useful YDB access to as many programming languages as possible through their established ODBC libraries. Select one representative framework or binding per language, run its upstream database tests when they exist, maintain an explicit YDB compatibility patch series for those tests, and provide a runnable example application in every selected language.

Languages with a maintained native YDB SDK are excluded because their native SDK is the preferred integration. The current exclusions are C++, Go, Java, Python, C#/.NET, JavaScript/TypeScript and Rust. PHP is the explicit exception because its native SDK is planned for deprecation. The exclusion list must be checked against the current [YDB SDK installation page](https://ydb.tech/docs/en/reference/ydb-sdk/install) whenever the framework matrix is updated.

## Acceptance criteria

- Every selected language has exactly one primary ODBC framework or binding, a pinned upstream revision, a reproducible YDB patch series, an Allure test result set and a repository-owned example application.
- Every upstream database test that exists is either executed, patched with a documented YDB-specific reason, or listed explicitly as not applicable; tests must never disappear silently.
- Framework implementation sources remain identical to the pinned upstream revision; YDB-specific changes are confined to test setup, fixtures and database adapters while preserving the original ODBC assertions.
- The existing driver unit and integration suites continue to pass.
- Driver-owned forward-only and static cursors provide standard ODBC fetch, scrolling, rowset binding and chunked-data behavior over YDB query results.
- Multiple independent ODBC connections can target different YDB databases and hosts without sharing sessions, transactions, credentials or catalog state.

## Language and framework matrix

The matrix is intentionally open-ended. A new language should be added whenever an installable Linux ODBC binding can connect through unixODBC, execute parameterized statements and fetch results. A small or old upstream suite is not a reason to reject a language; it means the shared contract and example application carry more of its coverage.

| Tier | Language | Selected framework or binding | Upstream tests to run | Required example |
|---|---|---|---|---|
| Core | Erlang | [OTP `odbc`](https://github.com/erlang/otp/tree/master/lib/odbc) | All YDB-applicable Common Test cases in `lib/odbc/test` | OTP application using `odbc:param_query` and transactions |
| Core | PHP | [PDO_ODBC](https://github.com/php/php-src/tree/master/ext/pdo_odbc) | PDO_ODBC PHPT tests and generic PDO tests selected by the existing harness | CLI application using `PDO`, prepared statements and transactions |
| Core | Haskell | [HDBC-odbc](https://github.com/hdbc/HDBC-odbc) | HDBC/HUnit database tests | Cabal application using prepared statements and `withTransaction` |
| Core | Ruby | [ruby-odbc](https://github.com/larskanis/ruby-odbc) | All upstream database-independent test scripts | Ruby application using prepared statements, iteration and rollback |
| Core | Lua | [LuaSQL ODBC](https://github.com/lunarmodules/luasql) | Common LuaSQL tests and ODBC-specific parameter tests | Lua application using environment, connection and cursor objects |
| Core | Perl | [DBD::ODBC](https://github.com/perl5-dbi/DBD-ODBC) | Generic TAP tests under `t/` | DBI application using binding, fetch hashes and transactions |
| Core | R | [`odbc` with DBI](https://github.com/r-dbi/odbc) | Package `testthat` tests and DBItest compliance groups | R script returning a typed data frame through DBI |
| Core | Julia | [ODBC.jl](https://github.com/JuliaDatabases/ODBC.jl) | ODBC.jl, DBInterface and Tables-compatible test sets | Julia application using `DBInterface.execute` and Tables rows |
| Core | Tcl | [`tdbc::odbc`](https://core.tcl-lang.org/tdbc) | `tcltest` suites for the ODBC backend | Tcl application using prepared statements and result-set iteration |
| Expansion | Raku | [DBDish::ODBC](https://github.com/salortiz/DBDish-ODBC) | Upstream `t/` tests and DBIish common tests supported by the adapter | Raku application using DBIish connection and statement handles |
| Expansion | Crystal | [crystal-odbc](https://github.com/naqvis/crystal-odbc) | Complete `crystal spec` suite | Crystal application using the `crystal-db` API |
| Expansion | Dart | [`dart_odbc`](https://pub.dev/packages/dart_odbc) | Complete `dart test` suite | Dart CLI application using prepared execution and typed rows |
| Expansion | D | [`odbc`](https://github.com/singingbush/odbc) | Upstream unit tests and integration-test executable | D application using the package connection and result APIs |
| Expansion | OCaml | [`ocaml-odbc`](https://opam.ocaml.org/packages/odbc/) | Upstream database tests when present; otherwise the shared contract | Dune application using prepared execution and row conversion |
| Expansion | Common Lisp | [CLSQL ODBC](https://github.com/sharplispers/clsql) | ODBC-applicable ASDF test systems when present; otherwise the shared contract | SBCL application using CLSQL query and transaction APIs |
| Expansion | COBOL | [GixSQL ODBC](https://github.com/mridoni/gixsql) | ODBC-capable GixSQL regression cases and examples | GnuCOBOL application using embedded SQL, a cursor and commit/rollback |
| Expansion | Pascal | [Free Pascal SQLDB ODBC](https://gitlab.com/freepascal.org/fpc/source/-/tree/main/packages/fcl-db) | FPCUnit SQLDB connector tests | Free Pascal application using `TODBCConnection`, `TSQLQuery` and `TSQLTransaction` |
| Expansion | Smalltalk | [Pharo-ODBC](https://github.com/pharo-rdbms/Pharo-ODBC) | Upstream SUnit tests | Headless Pharo example using connection, statement and result objects |
| Expansion | Fortran | [`odbc.f`](https://davidpfister.github.io/odbc.f/) | Upstream fpm tests when present; otherwise the shared contract | Fortran application using connection, result-set and column-set objects |

Core and Expansion jobs run only after changes are merged into `odbc-driver-feature` and when a tag is pushed. They do not run for pull requests or on a nightly schedule. Adding a new language requires one matrix row, not a second framework for a language already represented.

## YDB compatibility policy

The driver presents standard ODBC behavior and translates it to YDB semantics. This compatibility layer covers required table keys, namespace structure, supported types, DDL, common-table-expression syntax, identity behavior, result cursors and transaction modes. Applications and ODBC frameworks use their normal public APIs without YDB-specific source changes.

## Implementation details

### Driver architecture

The exported ODBC C ABI should remain in `src/odbc_driver.cpp`, while connection state, statement execution, descriptors, diagnostics and result-set handling remain in the existing handle classes. Compatibility behavior should be implemented in a new internal `src/compatibility/` module instead of being distributed across exported API functions.

`TStatement::ExecuteQuery()` should use the following pipeline:

1. Apply the existing ODBC escape translation unless `SQL_NOSCAN` is enabled.
2. Tokenize the statement while preserving string literals, quoted identifiers, comments and parameter markers.
3. Apply semantics-preserving YDB compatibility rewrites for table keys, common table expressions, identifiers and namespace resolution while preserving application-visible data.
4. Apply the existing `?` to `$pN` rewrite and generate typed `DECLARE` statements from the bound ODBC parameters.
5. Add the compatibility pragmas required by the statement and apply the current catalog with `TConnection::WrapQueryForCurrentCatalog()`.
6. Execute the final YQL through the Query Service and translate YDB status and result metadata back to ODBC diagnostics and types.

`SQLNativeSql` should run the same translation pipeline without executing the statement. This makes the API useful for diagnosing the exact YQL that the driver will submit and prevents it from disagreeing with `SQLPrepare` and `SQLExecDirect`.

### Primary-key emulation

For `CREATE TABLE`, the compatibility parser should preserve an explicit primary key. When the statement has no primary key, it should promote a declared non-null unique constraint or unique index to the YDB primary key. If no suitable unique key exists, it should add a collision-free UUID column such as `_ydb_odbc_row_id` as the physical YDB primary key.

The generated UUID column is an internal storage detail. The driver should populate it for inserts, preserve it for updates, use it to identify rows for deletes and omit it from `SELECT *`, `SQLColumns`, `SQLPrimaryKeys`, `SQLStatistics` and result metadata. Explicit column lists, parameter counts, ordinal positions and affected-row counts remain those of the application-visible schema. One shared table-mapping record should describe the logical columns, physical columns, selected key strategy and generated-column name so DDL rewriting, DML rewriting and metadata always agree.

The mapping should be recovered from YDB schema metadata and a driver-owned metadata table, allowing a new process or pooled connection to use tables created by an earlier connection. Tests should cover explicit keys, promoted composite unique keys, generated UUID keys, inserts with and without column lists, updates, deletes, `SELECT *`, aliases, metadata, reconnects and concurrent writers.

### Cursor emulation

YDB returns query results rather than server-side ODBC cursors. Each executed statement should therefore create a driver-owned cursor over the returned typed rows and column metadata. The cursor state machine consists of `before first`, `on row or rowset`, `after last` and `closed`; statement re-execution replaces the previous cursor, and statement close, cancellation and connection close release its resources.

The Core path should implement `SQLFetch` and `SQLFetchScroll(SQL_FETCH_NEXT)` as forward movement through that cursor. A static scrollable cursor should materialize a result snapshot and implement `SQL_FETCH_FIRST`, `LAST`, `PRIOR`, `ABSOLUTE` and `RELATIVE` by changing a logical row position. `SQL_ATTR_ROW_ARRAY_SIZE`, row-wise and column-wise binding, `SQL_ATTR_ROWS_FETCHED_PTR` and row-status arrays should operate on consecutive rows beginning at that position.

Rows should remain in YDB's typed representation until `SQLBindCol` or `SQLGetData` requests an ODBC C type. The cursor keeps a separate `SQLGetData` byte offset for every column of the current row, resets those offsets whenever the position changes and preserves the row until all chunked reads are complete. Cursor movement should produce the standard ODBC outcomes and diagnostics, including `SQL_NO_DATA`, `24000`, `HY010`, `HY106`, `01004` and conversion SQLSTATEs.

Forward-only cursors should consume rows incrementally. Static cursors should use a bounded in-memory row store with a statement-local spill file and an index of row offsets after the memory threshold is reached. `SQL_ATTR_MAX_ROWS` limits population of either store. Explicit commit or rollback closes open cursors consistently with the advertised `SQL_CB_CLOSE` behavior.

The initial advertised cursor types should be `SQL_CURSOR_FORWARD_ONLY` and read-only `SQL_CURSOR_STATIC`. `SQLSetCursorName` and `SQLGetCursorName` maintain the statement-local ODBC name. Capability reporting should be derived from the implemented fetch orientations, cursor attributes and concurrency mode. Integration tests should exercise empty and single-row results, large spilled results, every supported orientation and offset, row arrays, bound columns, chunked `SQLGetData`, truncation, nulls, re-execution, cancellation, transaction completion and multiple simultaneous statement cursors.

### WITH-clause translation

The compatibility parser should translate each non-recursive CTE to a collision-free YQL named expression. It must first collect every `$identifier` in the complete query, allocate a deterministic unused name such as `$_odbc_cte_s0_n0_<hash>`, and maintain a scope-aware mapping from the ANSI relation name to that generated expression. Multiple CTEs must be emitted in dependency order, table references must be rewritten only in the correct scope, and existing declared parameters or global YQL named expressions must remain unchanged.

The initial implementation must cover chained CTEs, multiple references to one CTE, nested subqueries, CTE column aliases, quoted identifiers and statements containing ODBC parameters. Recursive and data-modifying CTEs form the next compatibility milestone.

Tests must include keywords inside strings and comments, nested and shadowed CTE names, an existing `$cte` variable, multiple CTEs and failure diagnostics for unsupported recursive syntax.

### Catalog and directory mapping

The implementation should build on the current catalog support rather than introduce an independent schema model. An ODBC catalog is a normalized absolute YDB database or directory path, the schema component is empty, `/` is the catalog separator, and `SQL_ATTR_CURRENT_CATALOG` changes the path used by `PRAGMA TablePathPrefix`.

Qualified-name resolution and metadata filters must use one shared normalizer. It must handle quoted path components, absolute and current-catalog-relative table names, repeated separators and attempts to traverse above the configured database root. `SQLTables`, `SQLColumns`, `SQLPrimaryKeys`, `SQLStatistics` and query execution must resolve the same logical name to the same physical YDB path.

Schema support should use a tested directory alias layer on top of this mapping whenever a framework requires a non-empty schema.

### Multiple databases and hosts

Each `SQLHDBC` must own an endpoint, database path, credentials, TLS settings, clients, sessions, transaction and current catalog. A single `SQLHENV` may contain many independently configured connection handles targeting different databases on the same host or databases on different hosts. Statements always execute through their parent connection, and each connection has an independent lifecycle and failure boundary.

One connection string identifies one YDB discovery or load-balancer endpoint and one database. `SQLEndTran(SQL_HANDLE_ENV, ...)` applies commit or rollback independently to every connected `SQLHDBC` and reports the per-connection diagnostic chain.

Integration tests must cover two databases on one endpoint, two endpoints, concurrent queries, independent commit/rollback, isolated credentials and catalog state, failure of one endpoint, and driver-manager pooling keyed by the complete endpoint/database/credential identity.

### YDB, YQL and driver boundaries

The required physical primary key and hierarchical object namespace are YDB constraints. YQL named expressions, parameter declarations, identifier quoting and ANSI translation are language-level constraints. The driver compatibility layer owns key emulation, SQL translation, catalog mapping, cursor emulation, transaction behavior and SQLSTATE diagnostics.

Every failing framework test should be assigned to one of these boundaries in Allure. Portable ODBC behavior belongs in the driver compatibility layer. Database-specific setup belongs in the test fixture or database adapter. Server limitations remain visible with the exact affected behavior and YDB issue documented.

### Capability reporting

`SQLGetInfo` and `SQLGetFunctions` should be generated from a tested capability registry shared with the implementation. Each compatibility feature lands with execution tests, capability-reporting tests and the corresponding registry entry.

## Framework test implementation

### Repository layout

The repository should keep one self-contained integration directory per language:

```text
odbc/tests/frameworks/
  registry.yaml
  <language>/
    upstream.lock
    patches/
    test-manifest.yaml
    run-tests
    convert-results
    example/
odbc/tests/reporting/
```

`registry.yaml` is the source of truth for the CI matrix and records the language, framework, tier, runtime image, upstream URL, revision, archive checksum, patch directory, test command, native result format and example command. `upstream.lock` repeats the immutable source identity inside each integration directory so a language can be reproduced independently.

### Patch policy

Framework and binding implementation files are verified against the pinned upstream revision. YDB-specific patches are limited by path to test files, fixtures and test-only configuration, stored as ordered files under `<language>/patches/` and applied to a clean pinned checkout during the CI job.

Allowed patches include:

- Replace another database's vendor-specific setup SQL with equivalent YDB/YQL setup.
- Use YDB-supported types where the original type is vendor-specific and the test is not testing that exact ODBC type.
- Map flat schemas, temporary database names or database creation steps to isolated YDB directories.
- Adapt expected database-specific error text while preserving the expected SQLSTATE class and operation outcome.
- Mark a test not applicable when it requires a database feature YDB does not provide and the driver accurately reports that capability as unsupported.

CI enforces the allowed patch paths, the pinned implementation-source checksum, the original assertion count and a manifest entry for every changed or inapplicable test. Crash, hang and data-corruption outcomes remain failures.

Every patch file must have a matching manifest record containing a stable patch ID, affected upstream test IDs, category (`YDB_NAMESPACE`, `YQL_SYNTAX`, `YDB_TYPE`, `UNSUPPORTED_CAPABILITY` or `VENDOR_SPECIFIC`), rationale and link to the relevant YDB/YQL limitation. CI must verify the upstream checksum, run `git apply --check`, apply the ordered series and publish both the patch manifest and resulting tree hash.

### Shared test contract

Every framework runs its upstream database suite when one exists. Bindings with incomplete upstream integration coverage additionally run a repository-owned shared contract through the selected binding's public API.

The shared contract covers connection and disconnection, invalid connection diagnostics, multiple connections, direct execution, preparation and rebinding, scalar and tabular results, forward and static cursor movement, rowset binding, chunked reads, `NULL`, integer, floating-point, decimal, UTF-8, binary and date/time values, metadata, affected-row counts, commit, rollback, autocommit, concurrent independent connections, cleanup after errors and resource finalization. Cases for optional ODBC features run when capability discovery reports them as supported.

### Example applications

Every language directory must contain a small executable example and a README with exact dependency installation and run commands. The example accepts `YDB_ODBC_DSN` or `YDB_ODBC_CONNECTION_STRING`, creates an isolated table with a standard non-null unique `id`, performs a parameterized insert, reads and prints typed rows through the framework's cursor, demonstrates commit and rollback, and removes its table. It must use only the selected language framework's public API and must run in CI after the tests.

Examples should share the same logical `people(id, name, score, created_at)` schema while remaining idiomatic for their language. They are product artifacts, not test patches, and should be suitable for copying into user documentation.

### Native result and Allure contract

Every launcher must preserve the framework's native output and produce a normalized result containing the stable upstream test identifier, status, duration, stdout, stderr and setup/teardown phase. Native formats such as Common Test logs, PHPT output, TAP, JUnit/XML, FPCUnit XML or framework-specific text should be converted externally rather than by editing the upstream runner.

Allure results should use the hierarchy `ODBC / <language> / <framework> / <upstream group>` and include the framework version, runtime version, driver commit, YDB version, endpoint/database mode, upstream checksum, patched-tree hash and applied patch IDs. The history identifier must be derived from the language, pinned upstream revision and original upstream test identifier.

Failing tests should attach the SQLSTATE chain, native YDB issue text, translated YQL from `SQLNativeSql`, framework output and relevant server logs with secrets removed. Missing tests, an empty suite, patch-application failure, infrastructure failure or an unexpected skip should produce synthetic broken results and fail the job so a reduced test count cannot look like progress.

### CI workflow

A framework workflow should run only after changes are merged into `odbc-driver-feature` and when a tag is pushed. It must not run for pull requests, direct non-merge pushes, nightly or other scheduled events, or manual dispatches. Both triggers run the complete Core and Expansion matrix generated from `registry.yaml`, with the driver built once for the workflow.

Each job should start the same pinned YDB version, wait for readiness, create an isolated database prefix, register the driver in a job-local `odbcinst.ini`, fetch and verify upstream source, apply the reviewed patch series, run upstream tests, run the shared contract when required, run the example, and upload native plus Allure results even on failure. A final `if: always()` job validates manifests, merges results, builds the HTML report and publishes the raw results, rendered report, patch manifests and example logs.

The baseline may contain known failures while support is being implemented, but each post-merge or tag run must satisfy an incremental gate: no passing test or example regresses, no test disappears, the targeted behavior becomes passing, and upstream or patch changes are explicit. Once a language is green, its gate switches to zero failed, broken, missing or unexpected skipped cases.

The development loop is: inspect aggregated Allure failures, decide whether each failure is a driver defect or an unsound database assumption, add a focused driver regression test or a documented test patch, and rerun the affected language and existing ODBC suites locally before merging into `odbc-driver-feature`. The full matrix runs after that merge. Tag creation runs the same matrix against the tagged revision.

## Delivery order

1. Add `registry.yaml`, the framework directory template, source verification, patch verification, native-result conversion and Allure aggregation.
2. Extract the shared SQL compatibility pipeline and implement primary-key emulation, persistent logical-to-physical table mappings, `SQLNativeSql` translation and collision-safe non-recursive WITH translation.
3. Implement driver-owned forward-only and static cursors, bounded buffering and spill, all declared fetch orientations, rowset binding and chunked `SQLGetData`.
4. Onboard the Core languages with pinned upstream suites, reviewed YDB patches, test manifests and runnable examples.
5. Add the shared contract for bindings with incomplete upstream integration coverage.
6. Finish catalog normalization, per-connection authentication/TLS configuration and multiple-host/multiple-database isolation tests.
7. Onboard Expansion languages one at a time and add each stable job to the post-merge and tag matrix.
8. Resolve remaining Allure failures as driver fixes or reviewed database-specific test patches, with a focused regression test or patch rationale for every change.
9. Enable zero-regression gates for every stable language in the post-merge and tag workflow.

The final acceptance evidence is a commit-specific Allure report for every registered language, the original and patched upstream source identities, patch manifests, example logs, existing ODBC unit and integration results and multiple-endpoint results.
