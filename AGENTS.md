# Standalone YDB C++ SDK

This repository is generated from `ydb-platform/ydb`. Most SDK sources are imported from `ydb/public/sdk/cpp` and are overwritten by the import workflow.

- Make SDK source, public API, protocol, and behavior changes in `ydb-platform/ydb` first, and send the pull request there.
- Use this repository for standalone-only CMake, packaging, CI, dependency, release, and import-workflow changes.
- Do not patch imported SDK sources here unless the change is strictly required by the standalone build and cannot be made upstream.
