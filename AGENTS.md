# Standalone YDB C++ SDK

This repository is generated from `ydb-platform/ydb`. Most SDK sources are imported from `ydb/public/sdk/cpp` and are overwritten by the import workflow.

- Make SDK source, public API, protocol, and behavior changes in `ydb-platform/ydb` first, and send the pull request there.
- Use this repository for standalone-only CMake, packaging, CI, dependency, release, and import-workflow changes.
- Do not patch imported SDK sources here unless the change is strictly required by the standalone build and cannot be made upstream.
- When importing tests from `src/**/ut`, add every standalone-compatible test to the matching `tests/unit/**/CMakeLists.txt` or `tests/integration/**/CMakeLists.txt`. Register it with `add_ydb_test`, the appropriate CTest label, and all required standalone link targets.
- Before finishing an import or test-registration change, configure the test build, build every added test target, and run the registered tests with CTest. Do not leave imported tests unregistered merely because their upstream build metadata is unavailable; document and skip only tests that require components absent from the standalone repository.
