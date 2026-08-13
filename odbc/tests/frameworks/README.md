# ODBC consumer harness
Each registry entry runs in a digest-pinned image with the packaged driver.
Adapters declare commands; custom runners and checksum-locked archive frameworks are supported.
Qt source is downloaded only in CI under its GPL-3.0 Qt exception option; it is neither
vendored nor included in SDK artifacts. The adapter changes fixture SQL only and reports
every upstream database test function as passed or explicitly unsupported.
Validate with `python3 odbc/tests/frameworks/harness.py registry` and
`python3 -m unittest odbc.tests.test_integration_harness`.
