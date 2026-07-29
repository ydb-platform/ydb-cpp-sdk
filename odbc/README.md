# YDB ODBC Driver

ODBC driver for YDB.

## Requirements

- CMake 3.10 or higher
- C/C++ compiler with C11 and C++20 support
- YDB C++ SDK (build with `YDB_SDK_ODBC=ON`)
- unixODBC development packages (`unixodbc`, `unixodbc-dev` on Debian/Ubuntu)

Static dependencies under `~/ydb_deps` must be built with
`-DCMAKE_POSITION_INDEPENDENT_CODE=ON` when linking the shared ODBC driver. See the
main [README](../README.md) dependency install section.

## Build

```bash
cmake --preset release-test-clang
cmake --build build --target ydb-odbc -j$(nproc)
```

The shared library is produced as `build/odbc/libydb-odbc.so`.

## Install

```bash
cmake --install build --prefix /usr/local
```

Installs `libydb-odbc` and registers the driver in
`${prefix}/etc/odbcinst.d/ydb-odbc.ini`.
`odbc.ini` is not installed — create your own DSN (see below).

## Configuration

For `SQLConnect("YDB", ...)`, `isql -v YDB`, or `Driver=YDB`.

**`odbcinst.ini`** — driver registration (generated on build/install). Section `[YDB]` is the driver name used as `Driver=YDB` in connection strings and DSNs. `Driver` and `Setup` are the full path to `libydb-odbc.so`. Use `/etc/odbcinst.ini`, a file in `/etc/odbcinst.d/`, or set `ODBCSYSINI` to the directory that contains `odbcinst.ini`.

```ini
[YDB]
Description=YDB ODBC Driver
Driver=/path/to/libydb-odbc.so
Setup=/path/to/libydb-odbc.so
```

**`odbc.ini`** — DSN named `YDB`. In section `[YDB]`: `Driver` is the registered driver name, `Server` is the YDB endpoint, `Database` is the database path. Use `/etc/odbc.ini` or set `ODBCINI` to your file path.

```ini
[ODBC Data Sources]
YDB=YDB ODBC Driver

[YDB]
Driver=YDB
Server=localhost:2136
Database=/local
```

## Usage

Example of connecting via isql:
```bash
isql -v YDB
```

Example usage in C:
```c
SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt;

// Initialize environment
SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

// Connect
SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
SQLConnect(dbc, (SQLCHAR*)"YDB", SQL_NTS,
          (SQLCHAR*)"", SQL_NTS,
          (SQLCHAR*)"", SQL_NTS);

// Execute query
SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
SQLExecDirect(stmt, (SQLCHAR*)"SELECT * FROM mytable", SQL_NTS);

// Cleanup
SQLFreeHandle(SQL_HANDLE_STMT, stmt);
SQLDisconnect(dbc);
SQLFreeHandle(SQL_HANDLE_DBC, dbc);
SQLFreeHandle(SQL_HANDLE_ENV, env);
```

Alternatively, use `SQLDriverConnect` with a connection string (does not require DSN in odbc.ini):
```c
SQLCHAR connStr[] = "Driver=YDB;Endpoint=localhost:2136;Database=/local";
SQLDriverConnect(dbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
```

## Parameters

`?` placeholders are rewritten to `$p1`, `$p2`, ... with auto-generated `DECLARE $pN AS <type>?;`
from `SQLBindParameter` types. YDB-native `$pN` syntax also works.

## License

Apache License 2.0
