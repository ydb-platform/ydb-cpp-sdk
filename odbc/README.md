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
sudo odbcinst -i -d -f /usr/local/share/ydb-odbc/odbcinst.ini
```

This installs `libydb-odbc` and its unixODBC registration template. The
`ydb-odbc` Debian package runs `odbcinst` automatically during installation
and unregisters the driver when the package is removed. `odbc.ini` is not
installed or modified — create your own DSN (see below).

## Configuration

For `SQLConnect("YDB", ...)`, `isql -v YDB`, or `Driver=YDB`.

**`odbcinst.ini`** — driver registration template (generated on build/install).
Section `[YDB]` is the driver name used as `Driver=YDB` in connection strings
and DSNs. `Driver` and `Setup` are the full path to `libydb-odbc.so`. Register
the template with `odbcinst -i -d -f`; the Debian package does this for you.

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
AuthMode=Anonymous
```

`SQLDriverConnect` may also combine a DSN with explicit attributes. Values in
the connection string take precedence over values from the DSN. The user name
and password passed to `SQLConnect` take precedence over `User` and `Password`
in the DSN.

### Connection attributes

| Attribute | Meaning |
| --- | --- |
| `Endpoint` | YDB endpoint. `Server` is an alias. A `grpc://` prefix forces a plaintext connection; `grpcs://` enables TLS. |
| `Database` | YDB database path. |
| `DSN` | DSN section to load before applying the remaining connection-string attributes. |
| `AuthMode` | `Anonymous`, `Token`, `Static`, `Metadata`, `ServiceAccount`, `OAuth2`, or `Environment`. Values are case-insensitive. |
| `Token` | Access token for `Token` mode. `AccessToken` is an alias. |
| `User`, `Password` | Credentials for `Static` mode. `UID` and `PWD` are aliases. |
| `MetadataHost`, `MetadataPort` | Optional metadata service address for `Metadata` mode. |
| `ServiceAccountKeyFile` | Path to a service-account JSON key for `ServiceAccount` mode. `SaFile` is an alias. |
| `OAuth2KeyFile` | Path to an OAuth 2.0 token-exchange configuration file for `OAuth2` mode. |
| `IamEndpoint` | IAM gRPC endpoint for service-account authentication, or HTTP token endpoint override for OAuth 2.0 token exchange. |
| `RootCertificate` | Path to a PEM root-certificate file. `CaFile` is an alias. |
| `ClientCertificate`, `ClientPrivateKey` | Paths to the PEM client certificate and private key. They must be specified together. |

If `AuthMode` is omitted, the driver infers it from exactly one credential
family (`Token`, static user/password, metadata settings, service-account key,
or OAuth 2.0 key). With no credential attributes it uses `Anonymous`. Conflicting
families and incomplete credentials are rejected with SQLSTATE `28000`.
`Environment` uses the SDK's standard `YDB_*_CREDENTIALS` variables.

Unrecognized connection-string attributes are ignored after reporting SQLSTATE
`01S00`; `SQLDriverConnect` completes with `SQL_SUCCESS_WITH_INFO`. This allows
ODBC applications to supply tool-specific attributes such as `APP` or `WSID`.

Certificate attributes contain file paths, not inline PEM. The driver reads the
files while establishing the ODBC connection. Supplying certificates enables
TLS; certificates cannot be combined with an explicitly plaintext `grpc://`
endpoint.

Examples:

```text
Driver=YDB;Endpoint=grpcs://ydb.example.net:2135;Database=/production;AuthMode=Token;Token=...
DSN=YDB;AuthMode=Static;UID=app;PWD=secret
Driver=YDB;Endpoint=localhost:2136;Database=/local;AuthMode=ServiceAccount;SaFile=/run/secrets/sa.json;IamEndpoint=grpc://localhost:4284
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

For `INSERT`, `UPDATE`, `DELETE`, `UPSERT`, and `REPLACE`, `SQLRowCount`
returns the affected-row count reported by YDB query statistics. Counts from
executed parameter-array entries are summed; ignored entries are not counted.
For statements without an applicable count, it returns `-1`.

## Parameters

`?` placeholders are rewritten to `$p1`, `$p2`, ... with auto-generated `DECLARE $pN AS <type>?;`
from `SQLBindParameter` types. YDB-native `$pN` syntax also works.

## License

Apache License 2.0
