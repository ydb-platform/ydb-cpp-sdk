#!/bin/bash
set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_deb_directory>"
    exit 1
fi

DEB_DIR=$(realpath "$1")
SCRIPT_DIR=$(dirname "$(realpath "$0")")
SOURCE_DIR=$(realpath "$SCRIPT_DIR/..")
TEST_DIR=$(realpath "$SCRIPT_DIR/../tests/deb_package")

echo "Building test Docker image..."
docker build -t ydb-cpp-sdk-deb-test "$TEST_DIR"

echo "Running test container..."
docker run --rm \
    -v "$DEB_DIR:/deb_packages:ro" \
    -v "$SOURCE_DIR:/source:ro" \
    ydb-cpp-sdk-deb-test \
    bash -c '
set -e
apt-get update

if ! compgen -G "/deb_packages/yandex-googleapis-api-common-protos*.deb" > /dev/null; then
  echo "Building yandex-googleapis-api-common-protos .deb..."
  cmake -S /source/scripts/googleapis_deb -B /tmp/build_googleapis_deb \
    -DCMAKE_INSTALL_PREFIX=/usr/share/yandex
  cmake --build /tmp/build_googleapis_deb -j"$(nproc)"
  cmake --build /tmp/build_googleapis_deb --target package
  dpkg -i /tmp/build_googleapis_deb/*.deb
else
  dpkg -i /deb_packages/yandex-googleapis-api-common-protos*.deb
fi

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
'

echo "Test successful!"
