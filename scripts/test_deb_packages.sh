#!/bin/bash
set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_deb_directory>"
    exit 1
fi

DEB_DIR=$(realpath "$1")
SCRIPT_DIR=$(dirname "$(realpath "$0")")
TEST_DIR=$(realpath "$SCRIPT_DIR/../tests/deb_package")

echo "Building test Docker image..."
docker build --network host -t ydb-cpp-sdk-deb-test "$TEST_DIR"

echo "Running test container..."
docker run --rm --network host \
    -v "$DEB_DIR:/deb_packages:ro" \
    ydb-cpp-sdk-deb-test \
    bash -c "apt-get update && apt-get install -y /deb_packages/*.deb && mkdir build && cd build && cmake .. -DCMAKE_PREFIX_PATH='/usr/share/yandex' && make && ./test_app"

echo "Test successful!"