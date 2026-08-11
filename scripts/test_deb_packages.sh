#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_deb_directory>"
    exit 1
fi

DEB_DIR=$(realpath "$1")
SCRIPT_DIR=$(dirname "$(realpath "$0")")
TEST_DIR=$(realpath "$SCRIPT_DIR/../tests/deb_package")

shopt -s nullglob
for package in \
    yandex-googleapis-api-common-protos \
    libydb-cpp-dev \
    libydb-cpp-iam-dev \
    libydb-cpp-otel-metrics-dev \
    libydb-cpp-otel-tracing-dev; do
    files=("$DEB_DIR/${package}_"*.deb)
    if [ "${#files[@]}" -ne 1 ]; then
        echo "Expected exactly one ${package} package, found ${#files[@]}"
        exit 1
    fi
done

echo "Building test Docker image..."
docker build -t ydb-cpp-sdk-deb-test "$TEST_DIR"

echo "Running test container..."
docker run --rm \
    -v "$DEB_DIR:/deb_packages:ro" \
    ydb-cpp-sdk-deb-test \
    bash -c '
set -e
apt-get update
apt-get install -y /deb_packages/yandex-googleapis-api-common-protos_*.deb

configure_components() {
  local name="$1"
  local components="${2:-}"
  local package_name="${3:-ydb-cpp-sdk}"
  local build_dir="/component_test/build-${name}"

  if [ -n "$components" ]; then
    cmake -S /component_test -B "$build_dir" \
      -DCMAKE_PREFIX_PATH=/usr/share/yandex \
      -DYDB_TEST_PACKAGE_NAME="$package_name" \
      -DYDB_TEST_COMPONENTS="$components"
  else
    cmake -S /component_test -B "$build_dir" \
      -DCMAKE_PREFIX_PATH=/usr/share/yandex \
      -DYDB_TEST_PACKAGE_NAME="$package_name"
  fi
}

apt-get install -y /deb_packages/libydb-cpp-dev_*.deb
test -f /usr/share/yandex/lib/cmake/ydb-cpp-sdk/release/ydb-cpp-sdk-core-targets.cmake
test ! -e /usr/share/yandex/lib/cmake/ydb-cpp-sdk/release/ydb-cpp-sdk-iam-targets.cmake
test ! -e /usr/share/yandex/lib/cmake/ydb-cpp-sdk/release/ydb-cpp-sdk-otel-metrics-targets.cmake
test ! -e /usr/share/yandex/lib/cmake/ydb-cpp-sdk/release/ydb-cpp-sdk-otel-tracing-targets.cmake
configure_components default
configure_components core "Driver;Table;Topic"
cmake -S /component_test -B /component_test/build-predefined-rpc \
  -DCMAKE_PREFIX_PATH=/usr/share/yandex \
  -DYDB_TEST_PREDEFINED_RPC_TARGETS=ON

apt-get install -y /deb_packages/libydb-cpp-iam-dev_*.deb
test -f /usr/share/yandex/lib/cmake/ydb-cpp-sdk/release/ydb-cpp-sdk-iam-targets.cmake
configure_components iam "Iam;Driver;Table;Topic" "YDB-CPP-SDK"

apt-get install -y /deb_packages/libydb-cpp-otel-metrics-dev_*.deb
test -f /usr/share/yandex/lib/cmake/ydb-cpp-sdk/release/ydb-cpp-sdk-otel-metrics-targets.cmake
configure_components otel-metrics "OpenTelemetryMetrics;Driver"

apt-get install -y /deb_packages/libydb-cpp-otel-tracing-dev_*.deb
test -f /usr/share/yandex/lib/cmake/ydb-cpp-sdk/release/ydb-cpp-sdk-otel-tracing-targets.cmake
configure_components otel-tracing "OpenTelemetryTrace;Driver"

mkdir -p /test_project/build
cd /test_project/build
cmake -DCMAKE_PREFIX_PATH=/usr/share/yandex ..
make -j"$(nproc)"
./test_app

cmake -S /test_project -B /test_project/build-pkgconfig-grpc \
  -DCMAKE_PREFIX_PATH=/usr/share/yandex \
  -DYDB_TEST_GRPC_PKGCONFIG_FALLBACK=ON
cmake --build /test_project/build-pkgconfig-grpc --parallel "$(nproc)"
/test_project/build-pkgconfig-grpc/test_app
'

echo "Test successful!"
