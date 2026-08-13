# ODBC consumer harness
Every entry runs in a digest-pinned image with the packaged `ydb-odbc` driver.
Adapters provide a `Dockerfile` and declare commands in `registry.yaml`; custom
`run-tests` executables are also supported. Archive bindings add `upstream.lock`
and an executable `example`.
Tests receive connection, upstream, and result variables. Declare expected or unsupported test IDs, then run:
```bash
python3 odbc/tests/frameworks/harness.py registry
python3 -m unittest odbc.tests.test_integration_harness
```
