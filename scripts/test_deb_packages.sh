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

YDB_DEPS_DIR=$(realpath "$SCRIPT_DIR/../../ydb_deps")

echo "Running test container..."
docker run --rm --network host \
    -v "$DEB_DIR:/deb_packages:ro" \
    -v "$YDB_DEPS_DIR:/ydb_deps:ro" \
    ydb-cpp-sdk-deb-test \
    bash -c "apt-get update && apt-get install -y /deb_packages/*.deb && mkdir build && cd build && cmake .. -DCMAKE_PREFIX_PATH='/ydb_deps/absl;/ydb_deps/protobuf;/ydb_deps/grpc;/ydb_deps/base64;/ydb_deps/brotli;/ydb_deps/jwt-cpp' && make && ./test_app"

echo "Test successful!"