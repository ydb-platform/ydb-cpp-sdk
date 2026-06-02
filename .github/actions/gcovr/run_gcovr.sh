#!/usr/bin/env bash
set -euo pipefail

: "${SOURCE_ROOT:?SOURCE_ROOT is required}"
: "${BUILD_DIR:?BUILD_DIR is required}"
: "${OUTPUT_DIR:?OUTPUT_DIR is required}"
: "${GCOV_EXECUTABLE:=llvm-cov-16 gcov}"

mkdir -p "${OUTPUT_DIR}"

gcovr_args=(
  --root "${SOURCE_ROOT}"
  --object-directory "${BUILD_DIR}"
  --gcov-executable "${GCOV_EXECUTABLE}"
  --print-summary
  --json-summary "${OUTPUT_DIR}/summary.json"
  --html --html-details
  -o "${OUTPUT_DIR}/index.html"
  --cobertura "${OUTPUT_DIR}/cobertura.xml"
  --exclude '.*contrib/.*'
  --exclude '.*tests/.*'
  --exclude '.*/_deps/.*'
  --filter 'src/'
  --filter 'include/ydb-cpp-sdk/'
  --filter 'plugins/'
)

if [[ -n "${FAIL_UNDER_LINE:-}" && "${FAIL_UNDER_LINE}" != "0" ]]; then
  gcovr_args+=(--fail-under-line "${FAIL_UNDER_LINE}")
fi

gcovr "${gcovr_args[@]}"

if [[ -n "${GITHUB_STEP_SUMMARY:-}" && -f "${OUTPUT_DIR}/summary.json" ]]; then
  {
    echo "## Code coverage"
    echo ""
    echo "| Metric | Covered | Total | Percent |"
    echo "| --- | ---: | ---: | ---: |"
    jq -r '
      .line as $l
      | .branch as $b
      | .function as $f
      | "| Line | \($l.covered) | \($l.num) | \($l.percent // 0)% |",
        "| Branch | \($b.covered) | \($b.num) | \($b.percent // 0)% |",
        "| Function | \($f.covered) | \($f.num) | \($f.percent // 0)% |"
    ' "${OUTPUT_DIR}/summary.json"
  } >> "${GITHUB_STEP_SUMMARY}"
fi
