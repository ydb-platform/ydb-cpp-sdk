# Erlang ODBC Series Example

Minimal Erlang client for YDB ODBC. Mirrors the main scenario of the C++ `basic_example`: creates `series`, `seasons`, and `episodes` tables, fills them with test data, runs several queries, and drops the tables.

## Requirements

- Erlang/OTP with the `odbc` module
- unixODBC
- Built and registered YDB ODBC driver
- Running YDB instance reachable via the connection string

Verify driver registration:

```bash
odbcinst -q -d
```

## Running

By default, `Driver=YDB;Endpoint=localhost:2136;Database=/local;` is used.

```bash
make run
```

With a different connection string:

```bash
make run CONN='...'
```
