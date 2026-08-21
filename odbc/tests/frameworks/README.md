# ODBC consumer harness
Each registry entry runs in a digest-pinned image with the packaged driver.
Adapters declare commands; custom runners and checksum-locked archive frameworks are supported.
Qt source is downloaded only in CI under its GPL-3.0 Qt exception option; it is neither
vendored nor included in SDK artifacts. The adapter changes fixture SQL only and reports
every upstream database test function as passed or explicitly unsupported.

For discovered suites, `required` patterns define tests that must pass. Expected outcomes
are declared in top-level `classifications` groups in `registry.yaml`: `outcome` is either
`unsupported` or `skipped`, `reason` is the manually maintained text shown in Allure, and
`tests` maps consumer names to test patterns. The raw framework error remains in the Allure
trace. New tests covered by `required` need no registry update. Every exception pattern must
match a result, conflicting outcomes are rejected, and an entire suite cannot be classified
as an exception with a trailing `.*` pattern.

```yaml
classifications:
  - outcome: unsupported
    reason: Manually maintained Allure reason.
    tests:
      consumer-name: [consumer-name.*.suite.test]
```

Validate with `python3 odbc/tests/frameworks/harness.py registry` and
`python3 -m unittest odbc.tests.test_integration_harness`.
