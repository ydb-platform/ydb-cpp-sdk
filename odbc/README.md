# YDB ODBC Driver

ODBC driver for YDB.

## Requirements

- CMake 3.22 or higher
- C/C++ compiler with C11 and C++20 support
- YDB C++ SDK (build with `YDB_SDK_ODBC=ON`)
- Linux: unixODBC development packages and `odbcinst`
- macOS: iODBC development headers and the OpenLink iODBC SDK frameworks

Dependencies are fetched at the versions pinned by the standalone SDK build.

## Supported platforms

| Platform | Driver manager | Status |
| --- | --- | --- |
| Linux | unixODBC | Source build; Ubuntu 24.04 amd64 package and automated API/consumer tests |
| macOS | iODBC 3.52.16 | Source build matching the application architecture; verified on arm64 |
| Windows | — | Not currently packaged or validated |

## Build

```bash
cmake --preset release-clang -DYDB_SDK_ODBC=ON -DYDB_SDK_EXAMPLES=OFF
cmake --build build --target ydb-odbc --parallel
```

The shared library is `build/odbc/libydb-odbc.so` on Linux and
`build/odbc/libydb-odbc.dylib` on macOS.

## Linux installation

```bash
cmake --preset release-clang -DYDB_SDK_ODBC=ON -DYDB_SDK_EXAMPLES=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --target ydb-odbc --parallel
sudo cmake --install build --component ydb-odbc
sudo odbcinst -i -d -f /usr/local/share/ydb-odbc/odbcinst.ini
```

This installs `libydb-odbc` and its unixODBC registration template. The
`ydb-odbc` Debian package runs `odbcinst` automatically during installation
and unregisters the driver when the package is removed. `odbc.ini` is not
installed or modified — create your own DSN (see below).

## macOS installation

Use Homebrew iODBC to build the driver. Also install the current
[OpenLink iODBC SDK](https://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/Downloads),
which supplies the universal `iODBC.framework` and `iODBCinst.framework` under
`/Library/Frameworks`. The driver itself must contain the architecture used by
the client process.

Pin all ODBC paths so CMake cannot mix unixODBC libraries with iODBC headers.
The static installer, IDN, and OpenSSL libraries keep the installed driver free
of Homebrew runtime paths:

```bash
brew install libidn libiodbc openssl@3
IODBC_ROOT="$(brew --prefix libiodbc)"
IDN_ROOT="$(brew --prefix libidn)"
OPENSSL_ROOT="$(brew --prefix openssl@3)"
cmake --preset release-clang \
  -DYDB_SDK_ODBC=ON \
  -DYDB_SDK_EXAMPLES=OFF \
  -DODBC_CONFIG="${IODBC_ROOT}/bin/iodbc-config" \
  -DODBC_INCLUDE_DIR="${IODBC_ROOT}/include" \
  -DODBC_LIBRARY="${IODBC_ROOT}/lib/libiodbc.dylib" \
  -DYDB_ODBCINST_LIBRARY="${IODBC_ROOT}/lib/libiodbcinst.a" \
  -DIDN_LIBRARIES="${IDN_ROOT}/lib/libidn.a" \
  -DOPENSSL_ROOT_DIR="${OPENSSL_ROOT}" \
  -DOPENSSL_USE_STATIC_LIBS=ON \
  -DYDB_ODBC_INSTALL_LIBDIR=/Library/ODBC/YDB \
  -DYDB_ODBC_INSTALL_DATADIR=/Library/ODBC/YDB
cmake --build build --target ydb-odbc --parallel
sudo cmake --install build --component ydb-odbc
otool -L /Library/ODBC/YDB/libydb-odbc.dylib
```

The final `otool` output must not contain build-directory or Homebrew paths.

## Configuration

For `SQLConnect("YDB", ...)`, `isql -v YDB`, or `Driver=YDB`.

**`odbcinst.ini`** — driver registration template (generated on build/install).
Section `[YDB]` is the driver name used as `Driver=YDB` in connection strings
and DSNs. `Driver` and `Setup` are the full path to the platform driver library.
Register the template with `odbcinst -i -d -f`; the Debian package does this
for you.

```ini
[YDB]
Description=YDB ODBC Driver
Driver=/path/to/libydb-odbc.so
Setup=/path/to/libydb-odbc.so
```

**`odbc.ini`** — DSN named `YDB`. In section `[YDB]`, `Driver` is the
registered driver name or an absolute driver-library path, `Endpoint` (or its
`Server` alias) is the YDB endpoint, and `Database` is the database path. On
Linux use `~/.odbc.ini` or `/etc/odbc.ini`; `ODBCINI` can override the path.

```ini
[ODBC Data Sources]
YDB=YDB ODBC Driver

[YDB]
Driver=YDB
Server=localhost:2136
Database=/local
AuthMode=Anonymous
```

On macOS, merge the following sections into `/Library/ODBC/odbcinst.ini` and
`/Library/ODBC/odbc.ini` for a system DSN visible to sandboxed applications.
Do not overwrite existing sections for other drivers or data sources.

```ini
; /Library/ODBC/odbcinst.ini
[ODBC Drivers]
YDB ODBC Driver=Installed

[YDB ODBC Driver]
Description=YDB ODBC Driver
Driver=/Library/ODBC/YDB/libydb-odbc.dylib
Setup=/Library/ODBC/YDB/libydb-odbc.dylib
```

```ini
; /Library/ODBC/odbc.ini
[ODBC Data Sources]
YDB=YDB ODBC Driver

[YDB]
Driver=/Library/ODBC/YDB/libydb-odbc.dylib
Endpoint=grpc://localhost:2136
Database=/local
AuthMode=Anonymous
```

For a non-sandboxed per-user setup, the equivalent macOS files live under
`~/Library/ODBC`. Verify the DSN with
`"$(brew --prefix libiodbc)/bin/iodbctest" "DSN=YDB"`.

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

`?` placeholders are rewritten to `$p1`, `$p2`, ... with auto-generated
`DECLARE` statements derived from `SQLBindParameter` types. Null values use an
optional YDB type. YDB-native `$pN` syntax also works.

## License

Apache License 2.0
