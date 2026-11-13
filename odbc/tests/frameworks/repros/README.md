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
