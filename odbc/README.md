# YDB ODBC Driver

ODBC driver for YDB.

## Requirements

- CMake 3.10 or higher
- C/C++ compiler with C11 and C++20 support
- YDB C++ SDK
- unixODBC (for Linux/macOS)

## Build

```bash
cmake -DYDB_SDK_ODBC=1 --preset release-clang
cmake --build --preset default
```

## Configuration

1. Make sure the driver is registered:
```bash
odbcinst -q -d
```

2. Check available data sources:
```bash
odbcinst -q -s
```

3. Edit `/etc/odbc.ini` to configure the connection:
```ini
[YDB]
Driver=YDB
Description=YDB Database Connection
Server=your-server:port
Database=/path/to/database
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

## Parameters

Use names $p1, $p2, ... for parameter names

## License

Apache License 2.0
