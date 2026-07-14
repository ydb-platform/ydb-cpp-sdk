# ODBC Core conformance audit

This directory contains a Linux/headless audit for the ODBC 3.x **Core interface
conformance** contract. It deliberately tests only Core requirements; claiming
Level 1 or Level 2 is outside its scope. It is a regression and gap-finding
suite, not an official certification program.

The executable talks through unixODBC, not through driver internals. It checks:

- the declared interface level and the complete mandatory function bitmap;
- Driver Manager enumeration entry points;
- Core environment, connection, and statement attributes;
- parameter/row arrays, bind offsets, and conservative array capabilities;
- application descriptor assignment, binding, lifetime, and mandatory fields;
- pre-`SQLExecDirect` bindings and scalar data-at-execution sequencing;
- numeric/text conversion, partial `SQLGetData`, and diagnostic lifetime;
- transaction completion edge semantics;
- standard result shapes for the Core catalog functions.

The existing integration tests under `../integration` provide deeper behavioral
coverage for connection lifecycle, statement execution, binding and fetching,
data-at-execution, transactions, diagnostics, metadata, and cursor operations.

## Running

A YDB server must be available at `localhost:2136`, with database `/local`.

```bash
cmake --preset release-test-clang
cmake --build build --target odbc-core-conformance_it -j$(nproc)
ctest --test-dir build -L core-conformance --output-on-failure
```

Failures are intentional evidence of missing Core behavior. Do not convert a
mandatory failure into a skip unless the cited ODBC conformance contract says
the feature is optional or belongs to a higher level.

## Open-source solutions evaluated

Research was refreshed on 2026-07-14.

| Project | Evaluated revision | Useful parts | Why it was not vendored |
|---|---:|---|---|
| [unixODBC-Test](https://sourceforge.net/projects/unixodbc-test/) | SVN r19 (2018-03-08) | The closest generic corpus: AutoTests, TestFarm, and a `MyODBC3/funccore` API set | GPLv2; the broad AutoTest corpus is ODBC 2-era, requires Qt plus the ODBCTest GUI/gtrtst library, and is not a headless CTest suite. TestFarm's ODBC 3 Core directory contains only handle-allocation tests. |
| [Microsoft ODBCTest](https://github.com/microsoft/ODBCTest/tree/0d629c7e4ff7b01398a5ac71d20c43362d0f43bf) | `0d629c7e` | MIT-licensed interactive API exerciser and reference implementation | Visual Studio/Windows GUI application, interactive rather than an automated Linux conformance corpus. |
| [pyodbc](https://github.com/mkleehammer/pyodbc/tree/9fd386c370288d802ddec58e5580fe1ec9adeade) | `9fd386c3` | Mature real-client compatibility tests | Tests the Python DB-API wrapper and database-specific SQL, not raw driver Core conformance. |
| [nanodbc](https://github.com/nanodbc/nanodbc/tree/fd9b4f551b0f03780168c4b2ba880dcb5777aad4) | `fd9b4f55` | Portable C++ client smoke and type-conversion tests | Tests the nanodbc wrapper and is configured around a SQLite ODBC data source. |

The audit is implemented locally instead of copying those sources. That keeps
the repository Apache-2.0-only, avoids a GUI/runtime dependency, and makes every
assertion traceable to the current ODBC 3.x Core contract:

- [Core Interface Conformance](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/core-interface-conformance)
- [Function Conformance](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/function-conformance)
- [Attribute Conformance](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/attribute-conformance)
- [Descriptor Field Conformance](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/descriptor-field-conformance)
