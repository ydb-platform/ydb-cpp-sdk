# YDB ODBC Driver

ODBC driver for YDB.

## Requirements

- CMake 3.10 or higher
- C/C++ compiler with C11 and C++20 support
- YDB C++ SDK
- unixODBC (for Linux/macOS)

## Build

```bash
cmake -DYDB_SDK_ODBC=1 --preset release-test-clang
cmake --build --preset default
```

The shared library is produced as `build/odbc/libydb-odbc.so`.

## Install

After configure and build, CMake generates `build/odbc/odbcinst.ini` with the correct path to `libydb-odbc.so`.

Register the driver with unixODBC:

```bash
sudo odbcinst -i -d -f build/odbc/odbcinst.ini
```

Add a DSN — either copy the sample into the system config:

```bash
sudo cp odbc/odbc.ini /etc/odbc.ini
# edit Server, Database, etc.
```

or point applications at the sample in the repo:

```bash
export ODBCINI=/absolute/path/to/ydb-cpp-sdk/odbc/odbc.ini
```


## Configuration

1. Make sure the driver is registered:

```bash
odbcinst -q -d
```

You should see an entry named `YDB`.

2. Check available data sources:

```bash
odbcinst -q -s
```

3. Edit `/etc/odbc.ini` (or your `ODBCINI` file) to configure the connection:
```ini
[YDB]
Driver=YDB
Description=YDB Database Connection
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

Use names $p1, $p2, ... for parameter names

## License

Apache License 2.0
