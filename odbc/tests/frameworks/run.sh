#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -ne 4 ]]; then
    echo "Usage: $0 <consumer> <runtime-image> <ydb-odbc.deb> <results-dir>" >&2
    exit 2
fi

consumer="$1"
runtime="$2"
package="$(realpath "$3")"
results="$4"
root="$(cd "$(dirname "$0")" && pwd)"
adapter="${root}/${consumer}"
test -f "$package"
test -f "${adapter}/Dockerfile"
mkdir -p "$results"
results="$(realpath "$results")"
chmod 0777 "$results"
image="ydb-odbc-${consumer}:${GITHUB_RUN_ID:-local}-${GITHUB_RUN_ATTEMPT:-1}"

docker build --build-arg "RUNTIME_IMAGE=${runtime}" -t "$image" "$adapter"
docker run --rm --network host --user 0:0 \
    -v "${root}:/harness:ro" \
    -v "$(dirname "$package"):/packages:ro" \
    -v "${results}:/results" \
    -e "ODBC_PACKAGE_BASENAME=$(basename "$package")" \
    -e "ODBC_RUNTIME_IMAGE=${runtime}" \
    -e "DRIVER_COMMIT=${DRIVER_COMMIT:?}" \
    -e "YDB_TEST_IMAGE=${YDB_TEST_IMAGE:?}" \
    -e "YDB_ENDPOINT=${YDB_ENDPOINT:?}" \
    -e "YDB_DATABASE=${YDB_DATABASE:?}" \
    "$image" python3 /harness/harness.py run --consumer "$consumer"
