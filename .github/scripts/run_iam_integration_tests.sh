#!/usr/bin/env bash

set -euo pipefail

IAM_REGEX='^(DriverAuth|TMetadataFixture|TJwtIamFixture|TOAuthIamFixture|OAuth_WithFacility)\.'
IAM_CONTAINER_NAME="${IAM_CONTAINER_NAME:-ydb-iam}"
IAM_CTEST_JOBS="${IAM_CTEST_JOBS:-2}"
IAM_READY_ATTEMPTS="${IAM_READY_ATTEMPTS:-60}"
IAM_READY_SLEEP_SECONDS="${IAM_READY_SLEEP_SECONDS:-2}"
IAM_TOKEN="${IAM_TOKEN:-root@builtin}"

cleanup_iam() {
  docker rm -f "${IAM_CONTAINER_NAME}" >/dev/null 2>&1 || true
}

wait_for_iam_ydb() {
  for _ in $(seq 1 "${IAM_READY_ATTEMPTS}"); do
    if docker exec -e "YDB_TOKEN=${IAM_TOKEN}" "${IAM_CONTAINER_NAME}" /ydb \
        --endpoint grpc://localhost:2136 \
        --database /local \
        sql -s 'select 1' >/dev/null 2>&1; then
      return 0
    fi
    sleep "${IAM_READY_SLEEP_SECONDS}"
  done

  echo "ydb-iam failed to become ready"
  docker logs "${IAM_CONTAINER_NAME}" 2>&1 | tail -50 || true
  return 1
}

trap cleanup_iam EXIT
cleanup_iam

docker run -d --name "${IAM_CONTAINER_NAME}" --hostname localhost \
  -p 2235:2135 -p 2236:2136 -p 28765:8765 \
  -v /tmp/ydb_iam_certs:/ydb_certs \
  -e YDB_USE_IN_MEMORY_PDISKS=true \
  -e YDB_TABLE_ENABLE_PREPARED_DDL=true \
  -e YDB_ENFORCE_USER_TOKEN_REQUIREMENT=true \
  -e YDB_DEFAULT_CLUSTERADMIN=root@builtin \
  ghcr.io/ydb-platform/local-ydb:trunk

wait_for_iam_ydb

YDB_ENDPOINT=localhost:2236 YDB_DATABASE=/local \
  ctest -j"${IAM_CTEST_JOBS}" --test-dir build -R "${IAM_REGEX}" --output-on-failure
