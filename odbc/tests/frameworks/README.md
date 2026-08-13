# ODBC integration consumers

The registry in `registry.yaml` is the source of truth for the consumer matrix.
Each entry runs in its own digest-pinned image, installs the `ydb-odbc` Debian
package, connects to the same pinned YDB service, and publishes normalized plus
Allure results.

## Adding a consumer

1. Add an entry to `registry.yaml` and a directory named after its `id`.
2. Add a `Dockerfile` that accepts `RUNTIME_IMAGE` and installs only the runtime,
   build tools, and test framework needed by that consumer.
3. Add executable `runtime-version`, `run-tests`, and `convert-results` commands.
4. Declare every test in `expectations.yaml` as `required` or `unsupported`.
5. Run `python3 odbc/tests/frameworks/validate_registry.py` and
   `python3 -m unittest odbc/tests/test_integration_harness.py`.

Commands receive `YDB_ODBC_DSN`, `YDB_ODBC_CONNECTION_STRING`, `YDB_ENDPOINT`,
`YDB_DATABASE`, `ODBC_UPSTREAM_DIR`, and the result-directory variables from the
common runner. Tests execute as an unprivileged user and cannot access the SDK
build tree. `run-tests` must write `native/results.json` using normalized schema
version 1; `record_result.py` is the shared append helper. The converter should
normally invoke `reporting/convert_normalized.py`.

## Upstream binding consumers

Use `kind: binding` with an immutable source archive URL, revision, and SHA-256.
Copy those three fields exactly into `upstream.lock`. The common runner downloads,
verifies, safely extracts, and makes that source read-only. Provide an executable
`example` command that builds and runs the upstream project's example against the
installed driver without modifying upstream source. The runner records it as the
required `<consumer-id>.example` test, so add that ID to `expectations.yaml`.

The registry validator rejects mutable images, duplicate IDs, missing commands,
unlocked upstream sources, executable-bit mistakes, and incomplete binding
example contracts before the dynamic GitHub Actions matrix is created.
