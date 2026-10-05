#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
"${YDB_TECH_BUILD_DIR:-build}/ydb_tech/topic/ydb_tech_topic"
