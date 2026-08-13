#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -ne 4 ]]; then
    echo "Usage: $0 <consumer-id> <runtime-image> <ydb-odbc.deb> <results-dir>" >&2
    exit 2
fi

consumer_id="$1"
runtime_image="$2"
package_path="$(realpath "$3")"
results_dir="$4"
frameworks_dir="$(cd "$(dirname "$0")/.." && pwd)"
tests_dir="$(cd "${frameworks_dir}/.." && pwd)"
adapter_dir="${frameworks_dir}/${consumer_id}"

if [[ ! -f "$package_path" ]]; then
    echo "ODBC package does not exist: $package_path" >&2
    exit 1
fi
if [[ ! -f "${adapter_dir}/Dockerfile" ]]; then
    echo "Consumer adapter does not exist: $consumer_id" >&2
    exit 1
fi

mkdir -p "$results_dir"
results_dir="$(realpath "$results_dir")"
chmod 0777 "$results_dir"

image_name="ydb-odbc-consumer-${consumer_id}:${GITHUB_RUN_ID:-local}-${GITHUB_RUN_ATTEMPT:-1}"
docker build \
    --build-arg "RUNTIME_IMAGE=${runtime_image}" \
    --tag "$image_name" \
    --file "${adapter_dir}/Dockerfile" \
    "$adapter_dir"

docker run --rm --network host --user 0:0 \
    --volume "${tests_dir}:/harness:ro" \
    --volume "$(dirname "$package_path"):/packages:ro" \
    --volume "${results_dir}:/results" \
    --env "ODBC_CONSUMER_ID=${consumer_id}" \
    --env "ODBC_PACKAGE_BASENAME=$(basename "$package_path")" \
    --env "ODBC_RUNTIME_IMAGE=${runtime_image}" \
    --env "DRIVER_COMMIT=${DRIVER_COMMIT:?DRIVER_COMMIT is required}" \
    --env "YDB_TEST_IMAGE=${YDB_TEST_IMAGE:?YDB_TEST_IMAGE is required}" \
    --env "YDB_ENDPOINT=${YDB_ENDPOINT:?YDB_ENDPOINT is required}" \
    --env "YDB_DATABASE=${YDB_DATABASE:?YDB_DATABASE is required}" \
    "$image_name" \
    python3 /harness/frameworks/common/run_consumer.py
