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
mkdir -p "$MONOREPO/util"
mkdir -p "$MONOREPO/library/cpp/example"
mkdir -p "$MONOREPO/library/cpp/yt/threading/unittests"
mkdir -p "$MONOREPO/contrib/libs/libc_compat"
mkdir -p "$MONOREPO/tools/enum_parser"

echo "MONOREPO_TRACE" > "$MONOREPO/ydb/public/sdk/cpp/plugins/trace/otel/trace.cpp"
echo "MONOREPO_METRICS" > "$MONOREPO/ydb/public/sdk/cpp/plugins/metrics/otel/metrics.cpp"
echo "MONOREPO_AGENTS" > "$MONOREPO/ydb/public/sdk/cpp/AGENTS.md"
echo "# MONOREPO_PLUGINS_CMAKE" > "$MONOREPO/ydb/public/sdk/cpp/plugins/CMakeLists.txt"
echo "// monorepo type_switcher" > "$MONOREPO/ydb/public/sdk/cpp/include/ydb-cpp-sdk/type_switcher.h"
echo "// monorepo stlfwd" > "$MONOREPO/ydb/public/sdk/cpp/include/ydb-cpp-sdk/stlfwd.h"
echo "// monorepo version" > "$MONOREPO/ydb/public/sdk/cpp/src/version.h"
touch "$MONOREPO/ydb/public/api/client/yc_private/accessservice/sensitive.proto"
printf 'upstream=old\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/util/merged.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/util/renamed-old.txt"
echo "remove upstream" > "$MONOREPO/util/deleted.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/library/cpp/example/merged.txt"
echo "UPSTREAM_EVENT_COUNT" > "$MONOREPO/library/cpp/yt/threading/event_count.h"
echo "UPSTREAM_FUTEX" > "$MONOREPO/library/cpp/yt/threading/futex.cpp"
echo "UPSTREAM_YA_MAKE" > "$MONOREPO/library/cpp/yt/threading/ya.make"
echo "UPSTREAM_UNITTEST" > "$MONOREPO/library/cpp/yt/threading/unittests/event_count_ut.cpp"
printf 'upstream=old\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/contrib/libs/libc_compat/merged.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/tools/enum_parser/merged.txt"

git -C "$MONOREPO" init -q
git -C "$MONOREPO" config user.name "Import Test"
git -C "$MONOREPO" config user.email "import-test@example.com"
git -C "$MONOREPO" add .
git -C "$MONOREPO" commit -q -m "base"
BASE_COMMIT=$(git -C "$MONOREPO" rev-parse HEAD)

mkdir -p "$OSS"/{util,library,contrib,cmake,third_party,tools,.devcontainer,.github,scripts,examples}
mkdir -p "$OSS/library/cpp/example"
mkdir -p "$OSS/contrib/libs/libc_compat"
mkdir -p "$OSS/tools/enum_parser"
mkdir -p "$OSS/.git"
mkdir -p "$OSS/plugins/trace/otel"
mkdir -p "$OSS/plugins/metrics/otel"
mkdir -p "$OSS/include/ydb-cpp-sdk"
mkdir -p "$OSS/src"
mkdir -p "$OSS/tests/slo_workloads/key_value"
mkdir -p "$OSS/tests/deb_package/test_project"

echo "OSS_TRACE" > "$OSS/plugins/trace/otel/trace.cpp"
echo "OSS_METRICS" > "$OSS/plugins/metrics/otel/metrics.cpp"
echo "# OSS_CMAKE" > "$OSS/plugins/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/trace/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/metrics/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/trace/otel/CMakeLists.txt"
echo "# OSS_CMAKE" > "$OSS/plugins/metrics/otel/CMakeLists.txt"

touch "$OSS/.gitignore" "$OSS/CMakePresets.json" "$OSS/CMakeLists.txt"
touch "$OSS/codecov.yml"
touch "$OSS/LICENSE" "$OSS/README.md"
echo "OSS_AGENTS" > "$OSS/AGENTS.md"
touch "$OSS/tests/slo_workloads/.dockerignore" "$OSS/tests/slo_workloads/Dockerfile"
echo "OSS_SLO_WORKLOAD" > "$OSS/tests/slo_workloads/key_value/main.cpp"
echo "OSS_DEB_PACKAGE" > "$OSS/tests/deb_package/Dockerfile"
echo "OSS_DEB_PACKAGE" > "$OSS/tests/deb_package/test_project/main.cpp"
echo "// oss type_switcher" > "$OSS/include/ydb-cpp-sdk/type_switcher.h"
echo "// oss stlfwd" > "$OSS/include/ydb-cpp-sdk/stlfwd.h"
echo "// oss version" > "$OSS/src/version.h"
echo "$BASE_COMMIT" > "$OSS/.github/last_commit.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=custom\n' > "$OSS/util/merged.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=custom\n' > "$OSS/util/renamed-old.txt"
echo "remove upstream" > "$OSS/util/deleted.txt"
echo "standalone only" > "$OSS/util/local.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=custom\n' > "$OSS/library/cpp/example/merged.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=custom\n' > "$OSS/contrib/libs/libc_compat/merged.txt"
printf 'upstream=old\nseparator=unchanged\nstandalone=custom\n' > "$OSS/tools/enum_parser/merged.txt"

printf 'upstream=new\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/util/merged.txt"
git -C "$MONOREPO" mv util/renamed-old.txt util/renamed-new.txt
printf 'upstream=new\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/util/renamed-new.txt"
echo "added upstream" > "$MONOREPO/util/added.txt"
rm "$MONOREPO/util/deleted.txt"
printf 'upstream=new\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/library/cpp/example/merged.txt"
printf 'upstream=new\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/contrib/libs/libc_compat/merged.txt"
printf 'upstream=new\nseparator=unchanged\nstandalone=old\n' > "$MONOREPO/tools/enum_parser/merged.txt"
mkdir -p "$MONOREPO/library/cpp/not_vendored"
echo "must stay unvendored" > "$MONOREPO/library/cpp/not_vendored/file.txt"
git -C "$MONOREPO" add -A
git -C "$MONOREPO" commit -q -m "update util"

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
assert_contains "$OSS/AGENTS.md" "OSS_AGENTS" "MONOREPO_AGENTS"
assert_contains "$OSS/tests/slo_workloads/key_value/main.cpp" "OSS_SLO_WORKLOAD" ""
assert_contains "$OSS/tests/deb_package/Dockerfile" "OSS_DEB_PACKAGE" ""
assert_contains "$OSS/tests/deb_package/test_project/main.cpp" "OSS_DEB_PACKAGE" ""
assert_contains "$OSS/include/ydb-cpp-sdk/stlfwd.h" "oss stlfwd" "monorepo stlfwd"
assert_contains "$OSS/util/merged.txt" "upstream=new" ""
assert_contains "$OSS/util/merged.txt" "standalone=custom" ""
assert_contains "$OSS/util/added.txt" "added upstream" ""
assert_contains "$OSS/util/local.txt" "standalone only" ""
assert_contains "$OSS/util/renamed-new.txt" "upstream=new" ""
assert_contains "$OSS/util/renamed-new.txt" "standalone=custom" ""
assert_contains "$OSS/library/cpp/example/merged.txt" "upstream=new" ""
assert_contains "$OSS/library/cpp/example/merged.txt" "standalone=custom" ""
assert_contains "$OSS/library/cpp/yt/threading/event_count.h" "UPSTREAM_EVENT_COUNT" ""
assert_contains "$OSS/library/cpp/yt/threading/futex.cpp" "UPSTREAM_FUTEX" ""
assert_contains "$OSS/contrib/libs/libc_compat/merged.txt" "upstream=new" ""
assert_contains "$OSS/contrib/libs/libc_compat/merged.txt" "standalone=custom" ""
assert_contains "$OSS/tools/enum_parser/merged.txt" "upstream=new" ""
assert_contains "$OSS/tools/enum_parser/merged.txt" "standalone=custom" ""
if [ -e "$OSS/util/deleted.txt" ]; then
    echo "FAIL: upstream-deleted util file was retained" >&2
    failures=$((failures + 1))
fi
if [ -e "$OSS/util/renamed-old.txt" ]; then
    echo "FAIL: old path of upstream-renamed util file was retained" >&2
    failures=$((failures + 1))
fi
if [ -e "$OSS/library/cpp/not_vendored/file.txt" ]; then
    echo "FAIL: unvendored library component was imported" >&2
    failures=$((failures + 1))
fi
if [ -e "$OSS/library/cpp/yt/threading/ya.make" ]; then
    echo "FAIL: Arcadia build metadata was imported with the library component" >&2
    failures=$((failures + 1))
fi
if [ -e "$OSS/library/cpp/yt/threading/unittests/event_count_ut.cpp" ]; then
    echo "FAIL: unregistered upstream library tests were imported" >&2
    failures=$((failures + 1))
fi

while IFS= read -r cmake_file; do
    assert_contains "$cmake_file" "OSS_CMAKE" "MONOREPO_PLUGINS_CMAKE"
done < <(find "$OSS/plugins" -name "CMakeLists.txt" | sort)

if [ "$failures" -ne 0 ]; then
    echo "Plugin import script test failed ($failures assertion(s))." >&2
    exit 1
fi

echo "Plugin import script test passed."
