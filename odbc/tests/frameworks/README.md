# ODBC consumer harness

Every registry entry runs in a digest-pinned image using only the packaged
`ydb-odbc` driver. An adapter provides a `Dockerfile` and executable
`run-tests`; archive-backed bindings also provide `upstream.lock` and an
executable `example`. A `normalize-results` hook may convert an upstream test
format into `native/results.json` without patching upstream code.

Tests receive `YDB_ODBC_DSN`, `YDB_ODBC_CONNECTION_STRING`, `YDB_ENDPOINT`,
`YDB_DATABASE`, `ODBC_UPSTREAM_DIR`, and result-directory variables. Use
`case.sh` for simple command-based cases. Add expected or explicitly
unsupported test IDs to `registry.yaml`, then run:

```bash
python3 odbc/tests/frameworks/harness.py registry
python3 -m unittest odbc.tests.test_integration_harness
```
