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
  --lcov "${OUTPUT_DIR}/coverage.lcov"
  --exclude '.*contrib/.*'
  --exclude '.*tests/.*'
  --merge-lines
  --exclude '.*/_deps/.*'
  --merge-lines
  --filter 'src/'
  --filter 'include/ydb-cpp-sdk/'
  --filter 'plugins/'
)

if [[ -n "${FAIL_UNDER_LINE:-}" && "${FAIL_UNDER_LINE}" != "0" ]]; then
  gcovr_args+=(--fail-under-line "${FAIL_UNDER_LINE}")
fi

gcovr "${gcovr_args[@]}"

sed '/^BRDA:/d;/^BRF:/d;/^BRH:/d' \
  "${OUTPUT_DIR}/coverage.lcov" > "${OUTPUT_DIR}/coverage-codecov.lcov"
