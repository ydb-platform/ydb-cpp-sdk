#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_deb_directory>"
    exit 1
fi

DEB_DIR=$(realpath "$1")
SCRIPT_DIR=$(dirname "$(realpath "$0")")
SOURCE_DIR=$(realpath "$SCRIPT_DIR/..")
TEST_DIR=$(realpath "$SCRIPT_DIR/../tests/deb_package")
YDB_TEST_IMAGE="${YDB_TEST_IMAGE:-ydbplatform/local-ydb:25.2.1}"
YDB_TEST_CONTAINER="ydb-odbc-package-test-$$"

cleanup() {
    docker rm -f "$YDB_TEST_CONTAINER" >/dev/null 2>&1 || true
}
trap cleanup EXIT

shopt -s nullglob
for package in \
    yandex-googleapis-api-common-protos \
    libydb-cpp-dev \
    libydb-cpp-iam-dev \
    libydb-cpp-otel-metrics-dev \
    libydb-cpp-otel-tracing-dev \
    ydb-odbc; do
    files=("$DEB_DIR/${package}_"*.deb)
    if [ "${#files[@]}" -ne 1 ]; then
        echo "Expected exactly one ${package} package, found ${#files[@]}"
        exit 1
    fi
done

echo "Building test Docker image..."
docker build -t ydb-cpp-sdk-deb-test "$TEST_DIR"

echo "Starting local YDB ${YDB_TEST_IMAGE}..."
docker run -d --name "$YDB_TEST_CONTAINER" --network host \
    -e GRPC_TLS_PORT=2135 \
    -e GRPC_PORT=2136 \
    -e MON_PORT=8765 \
    -e YDB_DEFAULT_LOG_LEVEL=NOTICE \
    -e YDB_USE_IN_MEMORY_PDISKS=true \
    "$YDB_TEST_IMAGE" >/dev/null

for _ in $(seq 1 60); do
    if docker exec "$YDB_TEST_CONTAINER" /bin/sh -c \
        "/ydb -e grpc://localhost:2136 -d /local scheme ls" >/dev/null 2>&1; then
        break
    fi
    sleep 2
done
if ! docker exec "$YDB_TEST_CONTAINER" /bin/sh -c \
    "/ydb -e grpc://localhost:2136 -d /local scheme ls" >/dev/null 2>&1; then
    docker logs "$YDB_TEST_CONTAINER" || true
    echo "Local YDB did not become ready" >&2
    exit 1
fi

echo "Running test container..."
docker run --rm --network host \
    -v "$DEB_DIR:/deb_packages:ro" \
    -v "$SOURCE_DIR:/source:ro" \
    ydb-cpp-sdk-deb-test \
    bash -c '
set -euo pipefail
apt-get update
apt-get install -y /deb_packages/yandex-googleapis-api-common-protos_*.deb

odbc_packages=(/deb_packages/ydb-odbc_*.deb)
if [ "${#odbc_packages[@]}" -ne 1 ] || [ ! -f "${odbc_packages[0]}" ]; then
  echo "Expected exactly one ydb-odbc package, found: ${odbc_packages[*]}" >&2
  exit 1
fi
odbc_deb="${odbc_packages[0]}"
sdk_version="$(sed -nE '\''s/.*YDB_SDK_VERSION = "([0-9]+\.[0-9]+\.[0-9]+)".*/\1/p'\'' /source/src/version.h)"
package_version="$(dpkg-deb -f "$odbc_deb" Version)"
package_name="$(dpkg-deb -f "$odbc_deb" Package)"
package_arch="$(dpkg-deb -f "$odbc_deb" Architecture)"
package_depends="$(dpkg-deb -f "$odbc_deb" Depends)"
host_arch="$(dpkg --print-architecture)"
multiarch="$(dpkg-architecture -qDEB_HOST_MULTIARCH)"
driver_path="/usr/lib/${multiarch}/libydb-odbc.so"
driver_template="/usr/share/ydb-odbc/odbcinst.ini"

test "$package_name" = ydb-odbc
test "$package_version" = "$sdk_version"
test "$package_arch" = "$host_arch"
for dependency in odbcinst libodbcinst2 libc6; do
  if ! grep -Eq "(^|, )${dependency}([ (]|,|$)" <<<"$package_depends"; then
    echo "Missing ydb-odbc dependency ${dependency}: ${package_depends}" >&2
    exit 1
  fi
done

cat >/tmp/unrelated-odbcinst.ini <<EOF_UNRELATED
[UnrelatedPackageTest]
Description=Unrelated package test driver
Driver=/usr/lib/${multiarch}/libodbc.so.2
Setup=/usr/lib/${multiarch}/libodbcinst.so.2
EOF_UNRELATED
odbcinst -i -d -f /tmp/unrelated-odbcinst.ini

cat >/etc/odbc.ini <<EOF_ODBCINI
[ODBC Data Sources]
YDBPackageTest=YDB ODBC Driver

[YDBPackageTest]
Driver=YDB
Server=localhost:2136
Database=/local
EOF_ODBCINI
cat >/root/.odbc.ini <<EOF_USER_ODBCINI
[UserPackageTest]
Driver=UnrelatedPackageTest
EOF_USER_ODBCINI
sha256sum /etc/odbc.ini /root/.odbc.ini >/tmp/odbc-ini.sha256

verify_ydb_registration() {
  local registration
  registration="$(odbcinst -q -d -n YDB)"
  grep -Fx "Driver=${driver_path}" <<<"$registration"
  grep -Fx "Setup=${driver_path}" <<<"$registration"
  grep -Fx "UsageCount=1" <<<"$registration"
}

run_odbc_consumers() {
  local isql_output
  isql_output="$(printf "SELECT 42 AS value;\n" | isql -b -v YDBPackageTest)"
  echo "$isql_output"
  grep -Eq "(^|[^0-9])42([^0-9]|$)" <<<"$isql_output"
  /odbc_qt_test/build/ydb_odbc_qt_test \
    "Driver={YDB};Server=localhost:2136;Database=/local"
}

rm -rf /tmp/ydb-odbc-old
dpkg-deb --raw-extract "$odbc_deb" /tmp/ydb-odbc-old
sed -i "s/^Version: .*/Version: ${sdk_version}~package-test1/" \
  /tmp/ydb-odbc-old/DEBIAN/control
dpkg-deb --build /tmp/ydb-odbc-old /tmp/ydb-odbc-old.deb

apt-get install -y /tmp/ydb-odbc-old.deb
test -f "$driver_path"
test -f "$driver_template"
grep -Fx "Driver=${driver_path}" "$driver_template"
verify_ydb_registration
run_odbc_consumers

apt-get install -y "$odbc_deb"
test "$(dpkg-query -W -f='\''${Version}'\'' ydb-odbc)" = "$sdk_version"
verify_ydb_registration
run_odbc_consumers
sha256sum --check /tmp/odbc-ini.sha256

apt-get remove -y ydb-odbc
test ! -e "$driver_path"
test ! -e "$driver_template"
if odbcinst -q -d -n YDB >/dev/null 2>&1; then
  echo "YDB remained registered after package removal" >&2
  exit 1
fi
odbcinst -q -d -n UnrelatedPackageTest >/dev/null
sha256sum --check /tmp/odbc-ini.sha256

apt-get install -y "$odbc_deb"
verify_ydb_registration
odbcinst -u -d -n YDB
cat >/tmp/replacement-ydb-odbcinst.ini <<EOF_REPLACEMENT
[YDB]
Description=Replacement YDB ODBC driver
Driver=/usr/lib/${multiarch}/libodbc.so.2
Setup=/usr/lib/${multiarch}/libodbcinst.so.2
EOF_REPLACEMENT
odbcinst -i -d -f /tmp/replacement-ydb-odbcinst.ini
replacement_registration="$(odbcinst -q -d -n YDB)"
apt-get remove -y ydb-odbc
test "$(odbcinst -q -d -n YDB)" = "$replacement_registration"
odbcinst -u -d -n YDB

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
