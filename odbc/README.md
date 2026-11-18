# YDB ODBC driver

Linux uses unixODBC; Ubuntu 24.04 amd64 packages are available as CI artifacts. macOS uses iODBC and a source build matching the client architecture. Windows is not packaged or covered by CI.

## Install and build

Install an Ubuntu package; it registers the driver but leaves your DSNs unchanged:

```sh
sudo apt install ./ydb-odbc_*.deb
```

For a source build, use CMake 3.22+, Ninja and a C11/C++20 compiler. CPM fetches pinned SDK dependencies. On Ubuntu:

```sh
sudo apt install cmake ninja-build clang git pkg-config python3 ragel yasm \
  libssl-dev libidn11-dev unixodbc-dev unixodbc odbcinst
cmake --preset release-clang -DYDB_SDK_ODBC=ON -DYDB_SDK_EXAMPLES=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --target ydb-odbc --parallel
sudo cmake --install build --component ydb-odbc
sudo odbcinst -i -d -f /usr/local/share/ydb-odbc/odbcinst.ini
```

The build produces `build/odbc/libydb-odbc.so`. Use `release-gcc` for GCC. To build a Debian package instead, run `YDB_DEB_COMPONENTS=ydb-odbc ./scripts/build_cpack_deb_packages.sh` on Ubuntu 24.04; it installs build dependencies and writes packages to `build-deb/packages`.

On macOS, install Xcode command-line tools and the [OpenLink iODBC SDK](https://www.iodbc.org/dataspace/doc/iodbc/wiki/iodbcWiki/Downloads) frameworks in `/Library/Frameworks`. Pin iODBC paths to avoid mixing driver managers, and use static dependencies:

```sh
brew install cmake ninja pkg-config python ragel yasm libidn libiodbc openssl@3
IODBC_ROOT="$(brew --prefix libiodbc)"
IDN_ROOT="$(brew --prefix libidn)"
OPENSSL_ROOT="$(brew --prefix openssl@3)"
cmake --preset release-clang -DYDB_SDK_ODBC=ON -DYDB_SDK_EXAMPLES=OFF \
  -DODBC_CONFIG="${IODBC_ROOT}/bin/iodbc-config" -DODBC_INCLUDE_DIR="${IODBC_ROOT}/include" \
  -DODBC_LIBRARY="${IODBC_ROOT}/lib/libiodbc.dylib" \
  -DYDB_ODBCINST_LIBRARY="${IODBC_ROOT}/lib/libiodbcinst.a" \
  -DIDN_LIBRARIES="${IDN_ROOT}/lib/libidn.a" \
  -DOPENSSL_ROOT_DIR="${OPENSSL_ROOT}" -DOPENSSL_USE_STATIC_LIBS=ON \
  -DYDB_ODBC_INSTALL_LIBDIR=/Library/ODBC/YDB -DYDB_ODBC_INSTALL_DATADIR=/Library/ODBC/YDB
cmake --build build --target ydb-odbc --parallel
sudo cmake --install build --component ydb-odbc
```

This installs `/Library/ODBC/YDB/libydb-odbc.dylib` and registers the driver and a local `YDB` system DSN under `/Library/ODBC`. Check that `otool -L /Library/ODBC/YDB/libydb-odbc.dylib` contains no build-directory or Homebrew paths.

## Connect

For a Linux system DSN, add this to `/etc/odbc.ini` (or `~/.odbc.ini` for one user):

```ini
[YDB]
Driver=YDB
Endpoint=grpc://localhost:2136
Database=/local
AuthMode=Anonymous
```

Run `isql -v YDB`, or call `SQLConnect` with `YDB`. On macOS, the installer creates this DSN with the absolute driver path; connect with `"$(brew --prefix libiodbc)/bin/iodbctest" "DSN=YDB"`.

For `SQLDriverConnect`, pass a full connection string or a DSN with overrides:

```text
Driver=YDB;Endpoint=grpc://localhost:2136;Database=/local;AuthMode=Anonymous
Driver=YDB;Endpoint=grpcs://ydb.example.net:2135;Database=/production;AuthMode=Token;Token=...
DSN=YDB;AuthMode=Token;Token=...
```

On macOS, use `Driver=YDB ODBC Driver` or the absolute `.dylib` path. Connection-string attributes override DSN values. `Server` aliases `Endpoint`; `grpc://` uses plaintext and `grpcs://` enables TLS.

`AuthMode` also accepts `Static` (`UID`/`PWD`), `Metadata`, `ServiceAccount` (`ServiceAccountKeyFile`), `OAuth2` (`OAuth2KeyFile`) and `Environment` (SDK credential environment variables). Without credentials, authentication defaults to `Anonymous`. `RootCertificate` accepts a PEM file path; `ClientCertificate` and `ClientPrivateKey` must be supplied together. Certificates require TLS. Add `AnsiSQL=1` for SQL-generating clients that need the ANSI lexer and Unicode literals.

## Examples

Set `ODBC_CONNECTION_STRING` to a full connection string with `AnsiSQL=1` or `Driver=YDB;DSN=YDB`; each example also accepts it as its first argument. Each queries YDB and prints `42` and `Hello YDB` separated by a tab. PDO, Qt, Ruby, R and HDBC also demonstrate parameter binding. Use ruby-odbc 0.999993 and the LuaSQL/HDBC-odbc revisions and installation commands in the [Examples workflow](../.github/workflows/examples.yaml).

| Binding | Run from the repository root |
| --- | --- |
| PHP ext/odbc | `php odbc/examples/php-odbc/main.php` |
| PHP PDO_ODBC | `php odbc/examples/php-pdo/main.php` |
| Ruby ruby-odbc | `ruby odbc/examples/ruby/main.rb` |
| R DBI/odbc | `Rscript odbc/examples/r/main.R` |
| LuaSQL ODBC | `lua odbc/examples/lua/main.lua` |
| Haskell HDBC/HDBC-odbc | `runghc odbc/examples/haskell/main.hs` |
| Qt QODBC | `c++ -std=c++17 odbc/examples/qt/main.cpp -o /tmp/ydb-odbc-qt $(pkg-config --cflags --libs Qt6Core Qt6Sql) && /tmp/ydb-odbc-qt` |

`?` parameters derive their YDB types from `SQLBindParameter`; native `$pN` parameters also work. For DML, `SQLRowCount` reports affected rows from YDB statistics, summed across executed parameter-array entries; otherwise it returns `-1`.

Independent connections and autocommit statements can run concurrently; calls on one handle and statements in an explicit transaction execute in order. Cross-thread `SQLCancel` stops further attempts and discards pending results, but an in-flight RPC can still finish and writes can commit. Keep bound buffers alive until calls finish.

## CI coverage

[Native tests](../.github/workflows/tests.yaml) build with Clang and GCC and run ODBC unit, integration, authentication, parameter, metadata, transaction, affected-row, cancellation and concurrency tests against the configured YDB version matrix.

[Consumer integration](../.github/workflows/odbc-integration.yaml) installs the same Debian package in isolated jobs against `ydbplatform/local-ydb:trunk`. It runs upstream Qt, PHP ext/odbc, PDO_ODBC, Ruby, LuaSQL and R/DBItest suites in both DSN and connection-string modes, plus `isql` and package installation/reinstallation/removal checks. Prior-version upgrade checks require an actual older package. Reports retain raw failures and distinguish documented consumer defects and unsupported cases.

[Examples](../.github/workflows/examples.yaml) runs these programs directly in its own job and checks their exact output through DSNs and full connection strings.
