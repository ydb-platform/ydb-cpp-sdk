# ODBC consumer harness
Each registry entry runs in a digest-pinned image with the packaged driver.
Adapters declare commands; custom runners and locked archive bindings are supported.
Validate with `python3 odbc/tests/frameworks/harness.py registry` and
`python3 -m unittest odbc.tests.test_integration_harness`.
