# Consumer defect reproductions

Use the matrix's locked consumer sources without implementation patches.
These programs also run against independent ODBC drivers. References and exact
case/diagnostic matches are in `../registry.yaml`.

Install control drivers with `apt-get install libsqliteodbc odbc-postgresql`.
Provide a disposable PostgreSQL database in `PG_ODBC`, for example
`Driver=PostgreSQL Unicode;Server=localhost;Port=5432;Database=postgres;Uid=odbc;Pwd=odbc`.

## Qt 6.4.2

```sh
c++ qt-null.cpp -o qt-null $(pkg-config --cflags --libs Qt6Core Qt6Sql)
./qt-null 'Driver=SQLite3;Database=:memory:'
```

Observed: `value valid=0 null=1 type=invalid queryNull=1`.
The NULL should retain the QString type. QODBC's `qGetStringData()` returns
an invalid QVariant after receiving SQL_NULL_DATA.

```sh
c++ qt-tinyint.cpp -o qt-tinyint $(pkg-config --cflags --libs Qt6Core Qt6Sql) -lodbc
./qt-tinyint
./qt-tinyint 'Driver=YDB;Server=localhost:2136;Database=/local;AuthMode=Anonymous'
```

Both drivers advertise signed SQL_TINYINT and return `-128` through SQL_C_SLONG.
QODBC nevertheless exposes UInt. SQLite wraps its unsigned read to `4294967168`;
YDB correctly rejects it with `22003`, leaving Qt with an invalid value.
The separate SQLite Boolean-metadata case expects Bool; QODBC deliberately maps
SQL_BIT to Int/UInt. That convention is a binding limitation, not a driver defect.

```sh
c++ qt-timestamp.cpp -o qt-timestamp $(pkg-config --cflags --libs Qt6Core Qt6Sql) -lodbc
./qt-timestamp
./qt-timestamp 'Driver=YDB;Server=localhost:2136;Database=/local;AuthMode=Anonymous'
```

Both SQLite ODBC and YDB reproduce a one-hour shift for the `UTC+01:00` input.
The raw timestamp is `2150-01-05 14:00:00.123`: QODBC copied wall-clock fields
without normalizing the offset before sending the timezone-free ODBC structure.

## PHP PDO_ODBC 8.3.6

```sh
php pdo-lazy.php "$PG_ODBC"
```

Observed lengths: `600, -1, -1`; the last two accesses return NULL.
Each access should return the same 600-byte string. PDO_ODBC calls SQLGetData
again after exhausting the column, without caching it.
SQLite ODBC restarts this stream after SQL_NO_DATA and hides the defect; use PostgreSQL here.

## PDO_ODBC result exhaustion and reexecution

```sh
php pdo-rowset-reexecute.php 'Driver=SQLite3;Database=:memory:'
```

Expected: both executions expose one column and return `value=1`.
Observed with stock PHP 8.3.6: after `nextRowset()` returns false, execution succeeds
but `columnCount()` is zero and associative fetch returns an empty array. This
reproduces with SQLite ODBC as well as YDB. The equivalent ODBC C API sequence
returns the correct column count, name and value after reexecution.

[PDO core](https://github.com/php/php-src/blob/php-8.3.6/ext/pdo/pdo_stmt.c#L175)
clears its public column count. The
[ODBC adapter](https://github.com/php/php-src/blob/php-8.3.6/ext/pdo_odbc/odbc_stmt.c#L824)
returns on `SQL_NO_DATA` before freeing its internal columns, and its execute path
refreshes metadata only when those columns are absent. A unixODBC trace confirms
that the second successful `SQLExecute` has no `SQLNumResultCols`/`SQLDescribeCol`
refresh. The contract retains the original assertion and records this failure.

## Ruby ODBC 0.999993 UTF-8 extension

Build both extensions from the unmodified
[locked gem](https://rubygems.org/downloads/ruby-odbc-0.999993.gem), then:

```sh
ruby -I /path/to/ext/utf8 ruby-utf8-unicode.rb "$ODBC_CONNECTION"
```

Expected bytes for U+1F30D: `f09f8c8d`. The UTF-8 extension returns `f09f8c80`.
In the locked `ext/odbc.c`, `uc_from_utf` reads `str[4]` instead of the fourth
byte `str[3]` at line 1522. It also reverses the UTF-16 surrogate bit halves at
lines 1527–1528; `mkutf` reverses them again, so an apparent round trip can hide
an incorrect server value. The ANSI extension preserves the same UTF-8 bytes.
The supplementary-character assertion remains a failure; ordinary Unicode and
NULL bindings are checked independently.

## LuaSQL 491fea76, Lua 5.4, 64-bit unixODBC

Build the locked LuaSQL archive with `make odbc LUA_SYS_VER=5.4` and expose
`src/odbc.so` as `luasql/odbc.so` on LUA_CPATH.

```sh
lua5.4 lua-fetch.lua 'Driver=SQLite3;Database=:memory:'
```

Expected: `b, c, nil`, then `4294967296` and `-4294967297`.
Observed: `c, userdata, nil`, then incorrect integers with varying upper bytes.
Fetching the final 40-column NULL row can corrupt the Lua stack and abort the process.
The [upstream fix](https://github.com/lunarmodules/luasql/pull/219) explains both defects.
The matrix isolates cases and preserves debugger output for crashes.
Only these undefined-behavior cases allow intermittent passes; unmatched diagnostics still fail.

## R odbc 1.4.2, DBItest 1.8.0

Install the unmodified locked packages in R_LIBS_USER, then:

```sh
Rscript r-contract.R "$PG_ODBC" postgres-results
Rscript r-contract.R 'Driver=SQLite3;Database=control.sqlite' sqlite-results list_objects_features
Rscript r-bind-types.R "$PG_ODBC"
```

`r-contract.R` runs DBItest without YDB fixtures and writes every original
case outcome to `r-results.json`. The metadata probe leaves one ordinary table
visible; an empty catalog hides the identifier component-name mismatch.

Control runs reproduce the missing warnings/errors, invalid row-limit handling,
parameter-list validation, completion/affected-row state, POSIXlt error and missing
DBI reexports classified in the registry. The time round trips return seconds
instead of minutes. The focused binding probe returns `2.5` instead of `1.5` for
the same literal comparisons used by DBItest: factors become integer codes,
integer Dates lose their class, difftime units are ignored, and a timestamp
parameter does not match R's own quoted timestamp literal.

These exceptions do not cover YDB driver failures. R's three generic DDL cases
are separately classified as unavailable YDB type syntax: an interoperable
application must use the native names returned by
[SQLGetTypeInfo](https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/sql-minimum-grammar).
