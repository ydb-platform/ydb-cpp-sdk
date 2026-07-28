# Odbc driver
Odbc is a database connection layer, which gives users the opportunity to execute sql and interact with a database using a standardised C ABI. This document regulates how should the odbc driver for YDB be implemented, which functionality it is supposed to cover, performance issues and the acceptance criteria.

## Goal

The driver should provide useful YDB access to as many programming languages as possible through their established ODBC libraries. Select one representative framework or binding per language, run its upstream database tests when they exist, maintain an explicit YDB compatibility patch series for those tests, and provide a runnable example application in every selected language.

Languages with a maintained native YDB SDK are excluded because their native SDK is the preferred integration. The current exclusions are C++, Go, Java, Python, C#/.NET, JavaScript/TypeScript and Rust. PHP is the explicit exception because its native SDK is planned for deprecation. The exclusion list must be checked against the current [YDB SDK installation page](https://ydb.tech/docs/en/reference/ydb-sdk/install) whenever the framework matrix is updated.

## Acceptance criteria

- Every selected language has exactly one primary ODBC framework or binding, a pinned upstream revision, a reproducible YDB patch series, an Allure test result set and a repository-owned example application.
- Every upstream database test that exists is either executed, patched with a documented YDB-specific reason, or listed explicitly as not applicable; tests must never disappear silently.
- Framework patches may adapt database assumptions to YDB, but they must not change the framework implementation or weaken assertions for ODBC behavior that the driver advertises.
- The existing driver unit, integration and conformance suites continue to pass.
- Multiple independent ODBC connections can target different YDB databases and hosts without sharing sessions, transactions, credentials or catalog state.
- PHP PDO_ODBC performance does not regress relative to the pinned native PHP SDK. The target is for PDO_ODBC to outperform the deprecated SDK on equivalent operations.

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

Core jobs run on every pull request once their baseline is stable. Expansion jobs run nightly while being onboarded and are promoted to the pull-request matrix after they produce deterministic results. Adding a new language requires one matrix row, not a second framework for a language already represented.

## YDB compatibility policy

YDB requires a primary key for every table. The ODBC driver must not synthesize keys, inject hidden columns, rewrite application DML to maintain hidden values or hide physical columns from metadata. Applications using YDB are responsible for providing a sound key, and framework test fixtures should be patched to do the same.

YDB and YQL also differ from other relational databases in namespace structure, supported types, DDL, common-table-expression syntax, stored procedures, identity columns, result-set capabilities and transaction modes. These differences should be handled either by a semantics-preserving driver feature or by an explicit framework test patch when the test encodes a database-specific assumption rather than an ODBC requirement.

## Implementation details

### Driver architecture

The exported ODBC C ABI should remain in `src/odbc_driver.cpp`, while connection state, statement execution, descriptors, diagnostics and result-set handling remain in the existing handle classes. Compatibility behavior should be implemented in a new internal `src/compatibility/` module instead of being distributed across exported API functions.

`TStatement::ExecuteQuery()` should use the following pipeline:

1. Apply the existing ODBC escape translation unless `SQL_NOSCAN` is enabled.
2. Tokenize the statement while preserving string literals, quoted identifiers, comments and parameter markers.
3. Apply semantics-preserving YDB compatibility rewrites for common table expressions, identifiers and namespace resolution; do not change table keys or application data.
4. Apply the existing `?` to `$pN` rewrite and generate typed `DECLARE` statements from the bound ODBC parameters.
5. Add the compatibility pragmas required by the statement and apply the current catalog with `TConnection::WrapQueryForCurrentCatalog()`.
6. Execute the final YQL through the Query Service and translate YDB status and result metadata back to ODBC diagnostics and types.

`SQLNativeSql` should run the same translation pipeline without executing the statement. This makes the API useful for diagnosing the exact YQL that the driver will submit and prevents it from disagreeing with `SQLPrepare` and `SQLExecDirect`.

### Primary-key behavior

The driver should submit `CREATE TABLE` statements without inventing a physical schema. If a table has no primary key, it should return the YDB failure through the normal ODBC diagnostic chain with an appropriate SQLSTATE and the native YDB issue text. Driver tests should verify correct diagnostics, while every framework fixture patch should add a deterministic key and update its inserts and expected metadata consistently.

### WITH-clause translation

The compatibility parser should translate each non-recursive CTE to a collision-free YQL named expression. It must first collect every `$identifier` in the complete query, allocate a deterministic unused name such as `$_odbc_cte_s0_n0_<hash>`, and maintain a scope-aware mapping from the ANSI relation name to that generated expression. Multiple CTEs must be emitted in dependency order, table references must be rewritten only in the correct scope, and existing declared parameters or global YQL named expressions must remain unchanged.

The initial implementation must cover chained CTEs, multiple references to one CTE, nested subqueries, CTE column aliases, quoted identifiers and statements containing ODBC parameters. `WITH RECURSIVE`, data-modifying CTEs and unsupported materialization modifiers should remain explicit driver gaps until they have a semantics-preserving implementation.

Tests must include keywords inside strings and comments, nested and shadowed CTE names, an existing `$cte` variable, multiple CTEs and failure diagnostics for unsupported recursive syntax.

### Catalog and directory mapping

The implementation should build on the current catalog support rather than introduce an independent schema model. An ODBC catalog is a normalized absolute YDB database or directory path, the schema component is empty, `/` is the catalog separator, and `SQL_ATTR_CURRENT_CATALOG` changes the path used by `PRAGMA TablePathPrefix`.

Qualified-name resolution and metadata filters must use one shared normalizer. It must handle quoted path components, absolute and current-catalog-relative table names, repeated separators and attempts to traverse above the configured database root. `SQLTables`, `SQLColumns`, `SQLPrimaryKeys`, `SQLStatistics` and query execution must resolve the same logical name to the same physical YDB path.

If an upstream suite proves that a non-empty schema is required, schema support should be implemented as a tested directory alias layer on top of this mapping; returning inconsistent schema values only to satisfy metadata assertions is not acceptable.

### Multiple databases and hosts

Each `SQLHDBC` must own an endpoint, database path, credentials, TLS settings, clients, sessions, transaction and current catalog. A single `SQLHENV` may contain many independently configured connection handles targeting different databases on the same host or databases on different hosts. Statements always execute through their parent connection, and disconnecting or failing one connection must not affect another.

One connection string identifies one YDB discovery endpoint and one database. A multi-node YDB database should use its discovery or load-balancer endpoint rather than exposing a host list through ODBC. Cross-connection transactions are not atomic: `SQLEndTran(SQL_HANDLE_ENV, ...)` may iterate over connections, but it must not be described as a distributed transaction.

Integration tests must cover two databases on one endpoint, two endpoints, concurrent queries, independent commit/rollback, isolated credentials and catalog state, failure of one endpoint, and driver-manager pooling without returning a connection for the wrong endpoint/database pair.

### YDB, YQL and driver boundaries

The required physical primary key and hierarchical object namespace are YDB constraints. YQL named expressions, parameter declarations, identifier quoting and unsupported ANSI constructs are language-level constraints. Patching framework fixtures to use a valid YDB schema, implementing semantics-preserving SQL translation, mapping catalogs, preserving ODBC transaction semantics and returning correct SQLSTATE diagnostics are project responsibilities.

Every failing framework test should be assigned to one of these boundaries in Allure. A portable ODBC behavior should be implemented in the driver when it can be provided without falsifying YDB semantics. A database-specific fixture or assertion should be patched or marked not applicable with a precise reason. A case may be classified as server-blocked only when no sound driver implementation or fixture adaptation exists, and it must remain visible with the exact server limitation documented.

### Capability reporting

`SQLGetInfo` and `SQLGetFunctions` must describe implemented behavior, not intended behavior. Each compatibility feature should therefore land with both execution tests and capability-reporting tests. Unsupported procedures, multiple result sets, scrollable cursors, asynchronous execution and batch operations must continue to report unsupported until their complete API behavior is implemented.

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
odbc/tests/performance/php/
```

`registry.yaml` is the source of truth for the CI matrix and records the language, framework, tier, runtime image, upstream URL, revision, archive checksum, patch directory, test command, native result format and example command. `upstream.lock` repeats the immutable source identity inside each integration directory so a language can be reproduced independently.

### Patch policy

Upstream framework and binding implementation code must remain unchanged. Test files, test fixtures and test-only configuration may be patched when necessary to make the suite sound for YDB. Patches should be stored as ordered files under `<language>/patches/` and applied to a clean pinned checkout during the CI job; the repository must not maintain an opaque fork.

Allowed patches include:

- Add explicit primary keys to fixture DDL and update fixture inserts and expected key metadata consistently.
- Replace another database's vendor-specific setup SQL with equivalent YDB/YQL setup.
- Use YDB-supported types where the original type is vendor-specific and the test is not testing that exact ODBC type.
- Map flat schemas, temporary database names or database creation steps to isolated YDB directories.
- Adapt expected database-specific error text while preserving the expected SQLSTATE class and operation outcome.
- Mark a test not applicable when it requires a database feature YDB does not provide and the driver accurately reports that capability as unsupported.

Patches must not modify the framework or binding implementation, remove tests without a manifest entry, weaken assertions for advertised ODBC behavior, replace framework calls with direct YDB calls, turn a crash/hang/data corruption failure into an expected failure, or hide a driver regression behind a YDB limitation.

Every patch file must have a matching manifest record containing a stable patch ID, affected upstream test IDs, category (`YDB_PRIMARY_KEY`, `YDB_NAMESPACE`, `YQL_SYNTAX`, `YDB_TYPE`, `UNSUPPORTED_CAPABILITY` or `VENDOR_SPECIFIC`), rationale and link to the relevant YDB/YQL limitation. CI must verify the upstream checksum, run `git apply --check`, apply the ordered series and publish both the patch manifest and resulting tree hash.

### Shared test contract

Frameworks with an upstream database suite run that suite after applying the reviewed patches. Frameworks without a useful upstream integration suite run a repository-owned shared contract through the public API of the selected binding. The shared contract is not a replacement for upstream tests when upstream tests exist.

The shared contract covers connection and disconnection, invalid connection diagnostics, multiple connections, direct execution, preparation and rebinding, scalar and tabular results, `NULL`, integer, floating-point, decimal, UTF-8, binary and date/time values, metadata, affected-row counts, commit, rollback, autocommit, concurrent independent connections, cleanup after errors and resource finalization. Cases for optional ODBC features run only when capability discovery reports them as supported.

### Example applications

Every language directory must contain a small executable example and a README with exact dependency installation and run commands. The example accepts `YDB_ODBC_DSN` or `YDB_ODBC_CONNECTION_STRING`, creates an isolated table with an explicit primary key, performs a parameterized insert, reads and prints typed rows, demonstrates commit and rollback, and removes its table. It must use only the selected language framework's public API and must run in CI after the tests.

Examples should share the same logical `people(id, name, score, created_at)` schema while remaining idiomatic for their language. They are product artifacts, not test patches, and should be suitable for copying into user documentation.

### Native result and Allure contract

Every launcher must preserve the framework's native output and produce a normalized result containing the stable upstream test identifier, status, duration, stdout, stderr and setup/teardown phase. Native formats such as Common Test logs, PHPT output, TAP, JUnit/XML, FPCUnit XML or framework-specific text should be converted externally rather than by editing the upstream runner.

Allure results should use the hierarchy `ODBC / <language> / <framework> / <upstream group>` and include the framework version, runtime version, driver commit, YDB version, endpoint/database mode, upstream checksum, patched-tree hash and applied patch IDs. The history identifier must be derived from the language, pinned upstream revision and original upstream test identifier.

Failing tests should attach the SQLSTATE chain, native YDB issue text, translated YQL from `SQLNativeSql`, framework output and relevant server logs with secrets removed. Missing tests, an empty suite, patch-application failure, infrastructure failure or an unexpected skip should produce synthetic broken results and fail the job so a reduced test count cannot look like progress.

### CI workflow

A framework workflow should run for pull requests targeting `odbc-driver-feature`, pushes to that branch, nightly schedules and manual dispatches. It should build the driver once and generate its matrix from `registry.yaml`. Core languages run on every pull request; all Core and Expansion languages run nightly and on manual full-matrix requests.

Each job should start the same pinned YDB version, wait for readiness, create an isolated database prefix, register the driver in a job-local `odbcinst.ini`, fetch and verify upstream source, apply the reviewed patch series, run upstream tests, run the shared contract when required, run the example, and upload native plus Allure results even on failure. A final `if: always()` job validates manifests, merges results, builds the HTML report and publishes the raw results, rendered report, patch manifests and example logs.

The baseline may contain known failures while support is being implemented, but each pull request must satisfy an incremental gate: no passing test or example regresses, no test disappears, the targeted behavior becomes passing, and upstream or patch changes are explicit. Once a language is green, its gate switches to zero failed, broken, missing or unexpected skipped cases.

The development loop is: inspect aggregated Allure failures, decide whether each failure is a driver defect or an unsound database assumption, add a focused driver regression test or a documented test patch, rerun the affected language and existing ODBC suites, then merge into `odbc-driver-feature`. The branch must be rebased and the applicable matrix rerun when its base advances.

## Performance test implementation

The PHP comparison must use one repository-owned workload implementation with two thin backends: native PHP SDK `ExecuteQuery` and PDO_ODBC. Both backends must use the same PHP runtime, YDB server, schema, seed data, query text, parameter values, connection lifetime, concurrency schedule, retry policy and result validation.

The benchmark should separate operation types rather than compare unlike APIs:

- Result-returning statements: native `ExecuteQuery` versus PDO `prepare`/`execute` plus the same row fetch and decoding work.
- Non-result statements: native `ExecuteQuery` versus PDO `exec` or prepared execution with identical transaction semantics.
- Connection setup: measured separately from steady-state execution so pooling and driver initialization costs remain visible.

The workload should include point reads, parameterized range reads, inserts, updates and an explicit transaction. It should run a warm-up phase followed by at least five paired 600-second samples in alternating backend order on the same isolated runner. Rate limiting, concurrency and data-set size must be explicit inputs and recorded in the result.

The workflow should publish requests per second, p50/p95/p99/p99.9 latency, error rate, retry count, CPU time per operation and peak resident memory. Each operation must validate returned row counts or affected-row counts so a faster error or empty result cannot be reported as an improvement.

The regression gate should compare paired samples with confidence intervals. PHP PDO_ODBC passes when throughput is not lower and p95/p99 latency is not higher than the pinned native PHP SDK by more than the agreed tolerance; the performance target is for ODBC to outperform the deprecated SDK, but correctness and a statistically stable no-regression gate come first. Any intentional change to the SDK version, YDB version, runner class or workload invalidates the stored baseline and requires a new reviewed baseline.

## Delivery order

1. Add `registry.yaml`, the framework directory template, source verification, patch verification, native-result conversion and Allure aggregation.
2. Onboard the Core languages with pinned upstream suites, reviewed YDB patches, test manifests and runnable examples.
3. Add the shared contract for bindings with incomplete upstream integration coverage.
4. Extract the shared SQL compatibility pipeline, make `SQLNativeSql` use it and implement collision-safe non-recursive WITH translation.
5. Finish catalog normalization, per-connection authentication/TLS configuration and multiple-host/multiple-database isolation tests.
6. Onboard Expansion languages one at a time and promote each stable job into the pull-request matrix.
7. Resolve remaining Allure failures as driver fixes or reviewed database-specific test patches, with a focused regression test or patch rationale for every change.
8. Add the paired PHP performance workflow and establish the reviewed native-SDK baseline.
9. Enable zero-regression gates for every stable language and the PHP performance gate on `odbc-driver-feature`.

The final acceptance evidence is a commit-specific Allure report for every registered language, the original and patched upstream source identities, patch manifests, example logs, existing ODBC unit/integration/conformance results, multiple-endpoint results and the paired PHP performance report. A result is not acceptable if implementation code in a selected framework was modified, a test was omitted without a manifest record, a patch masks advertised ODBC behavior, capability reporting overstates the driver, an example does not run, or the benchmark compares different semantics.
