#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(dirname "$(realpath "$0")")
REPO_ROOT=$(realpath "$SCRIPT_DIR/..")
COPY_SOURCES="$REPO_ROOT/.github/scripts/copy_sources.sh"

if [ ! -x "$COPY_SOURCES" ]; then
    echo "Error: copy_sources.sh not found or not executable: $COPY_SOURCES" >&2
    exit 1
fi

TMP_ROOT=$(mktemp -d)
MONOREPO="$TMP_ROOT/monorepo"
OSS="$TMP_ROOT/oss"

cleanup() {
    rm -rf "$TMP_ROOT"
}
trap cleanup EXIT

mkdir -p "$MONOREPO/ydb/public/sdk/cpp/src/client/"{arrow,cms,config,debug,draft}
mkdir -p "$MONOREPO/ydb/public/sdk/cpp/include/ydb-cpp-sdk/client/"{arrow,cms,config,debug,draft}
mkdir -p "$MONOREPO/ydb/public/sdk/cpp/tests/unit/client/draft"
mkdir -p "$MONOREPO/ydb/public/sdk/cpp/plugins/trace/otel"
mkdir -p "$MONOREPO/ydb/public/sdk/cpp/plugins/metrics/otel"
mkdir -p "$MONOREPO/ydb/public/api/client/yc_private/accessservice"
mkdir -p "$MONOREPO/ydb/public/api/client/yc_private/iam"
mkdir -p "$MONOREPO/ydb/public/api/client/yc_private/operation"
mkdir -p "$MONOREPO/ydb/public/api/client/yc_public/common"
mkdir -p "$MONOREPO/ydb/public/api/client/yc_public/iam"
mkdir -p "$MONOREPO/ydb/public/api/grpc"
mkdir -p "$MONOREPO/ydb/public/api/protos/out"

echo "MONOREPO_TRACE" > "$MONOREPO/ydb/public/sdk/cpp/plugins/trace/otel/trace.cpp"
echo "MONOREPO_METRICS" > "$MONOREPO/ydb/public/sdk/cpp/plugins/metrics/otel/metrics.cpp"
echo "# MONOREPO_PLUGINS_CMAKE" > "$MONOREPO/ydb/public/sdk/cpp/plugins/CMakeLists.txt"
echo "// monorepo type_switcher" > "$MONOREPO/ydb/public/sdk/cpp/include/ydb-cpp-sdk/type_switcher.h"
echo "// monorepo version" > "$MONOREPO/ydb/public/sdk/cpp/src/version.h"
touch "$MONOREPO/ydb/public/api/client/yc_private/accessservice/sensitive.proto"

mkdir -p "$OSS"/{util,library,contrib,cmake,third_party,tools,.devcontainer,.github,scripts,examples}
mkdir -p "$OSS/.git"
mkdir -p "$OSS/plugins/trace/otel"
mkdir -p "$OSS/plugins/metrics/otel"
mkdir -p "$OSS/include/ydb-cpp-sdk"
mkdir -p "$OSS/src"
mkdir -p "$OSS/tests/slo_workloads"

echo "OSS_TRACE" > "$OSS/plugins/trace/otel/trace.cpp"
echo "OSS_METRICS" > "$OSS/plugins/metrics/otel/metrics.cpp"
echo "# OSS_CMAKE" > "$OSS/plugins/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/trace/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/metrics/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/trace/otel/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/metrics/otel/CMakeLists.txt"

touch "$OSS/.gitignore" "$OSS/.gitmodules" "$OSS/CMakePresets.json" "$OSS/CMakeLists.txt"
touch "$OSS/LICENSE" "$OSS/README.md"
touch "$OSS/tests/slo_workloads/.dockerignore" "$OSS/tests/slo_workloads/Dockerfile"
echo "// oss type_switcher" > "$OSS/include/ydb-cpp-sdk/type_switcher.h"
echo "// oss version" > "$OSS/src/version.h"

"$COPY_SOURCES" "$MONOREPO" "$OSS"

failures=0

assert_contains() {
    local file=$1
    local expected=$2
    local forbidden=$3

    if [ ! -f "$file" ]; then
        echo "FAIL: missing file: $file" >&2
        failures=$((failures + 1))
        return
    fi

    if ! grep -q "$expected" "$file"; then
        echo "FAIL: $file does not contain '$expected'" >&2
        failures=$((failures + 1))
    fi

    if [ -n "$forbidden" ] && grep -q "$forbidden" "$file"; then
        echo "FAIL: $file still contains '$forbidden'" >&2
        failures=$((failures + 1))
    fi
}

assert_contains "$OSS/plugins/trace/otel/trace.cpp" "MONOREPO_TRACE" "OSS_TRACE"
assert_contains "$OSS/plugins/metrics/otel/metrics.cpp" "MONOREPO_METRICS" "OSS_METRICS"

while IFS= read -r cmake_file; do
    assert_contains "$cmake_file" "OSS_CMAKE" "MONOREPO_PLUGINS_CMAKE"
done < <(find "$OSS/plugins" -name "CMakeLists.txt" | sort)

if [ "$failures" -ne 0 ]; then
    echo "Plugin import script test failed ($failures assertion(s))." >&2
    exit 1
fi

echo "Plugin import script test passed."
