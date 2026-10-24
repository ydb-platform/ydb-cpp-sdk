#!/usr/bin/env bash
set -euo pipefail
cd "$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
"${YDB_TECH_BUILD_DIR:-build}/examples/ydb_tech/topic/ydb_tech_topic"
