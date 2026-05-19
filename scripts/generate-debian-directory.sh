#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
SOURCE_DIR=$(realpath "$SCRIPT_DIR/..")

cd "$SOURCE_DIR"

VERSION=$(grep -E 'YDB_SDK_VERSION = "[0-9]+\.[0-9]+\.[0-9]+"' src/version.h | sed -E 's/.*"([0-9]+\.[0-9]+\.[0-9]+)".*/\1/')
if [ -z "$VERSION" ]; then
    echo "Error: Could not extract YDB_SDK_VERSION from src/version.h"
    exit 1
fi

GIT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")

mkdir -p debian/

CHANGELOG_FILE="debian/changelog"

DEB_VERSION="${VERSION}-1"

cat <<EOF_CHANGELOG > "$CHANGELOG_FILE"
ydb-cpp-sdk (${DEB_VERSION}) unstable; urgency=low

  * Automated package build
  * Git commit: ${GIT_COMMIT}

 -- YDB Team <whoami@where>  $(date -R)
EOF_CHANGELOG

cat <<EOF_CONTROL > debian/control
Source: ydb-cpp-sdk
Section: libdevel
Priority: optional
Maintainer: YDB Team <whoami@where>
Build-Depends: debhelper-compat (= 13),
 cmake,
 git,
 pkg-config,
 python3,
 python3-six,
 ragel,
 yasm,
 libidn11-dev,
 libssl-dev,
 zlib1g-dev,
 libprotobuf-dev,
 protobuf-compiler,
 libgrpc++-dev,
 protobuf-compiler-grpc,
 libbrotli-dev,
 liblz4-dev,
 libzstd-dev,
 libbz2-dev,
 libxxhash-dev,
 libsnappy-dev,
 libdouble-conversion-dev,
 libgtest-dev,
 libre2-dev,
 libc-ares-dev,
 rapidjson-dev,
 yandex-googleapis-api-common-protos
Standards-Version: 4.6.2
Homepage: https://ydb.tech
Rules-Requires-Root: no

Package: libydb-cpp-dev
Architecture: any
Depends: \${misc:Depends},
 libidn11-dev,
 libssl-dev,
 zlib1g-dev,
 libprotobuf-dev,
 libgrpc++-dev,
 libbrotli-dev,
 liblz4-dev,
 libzstd-dev,
 libbz2-dev,
 libxxhash-dev,
 libsnappy-dev,
 libdouble-conversion-dev,
 libgtest-dev,
 libre2-dev,
 libc-ares-dev,
 rapidjson-dev,
 yandex-googleapis-api-common-protos
Description: YDB C++ SDK development files
 Static library, public headers and CMake package files for YDB C++ SDK.

Package: libydb-cpp-iam-dev
Architecture: any
Depends: \${misc:Depends}, libydb-cpp-dev (= \${binary:Version})
Description: YDB C++ SDK IAM plugin development files
 Static library for YDB C++ SDK IAM credentials plugin.

Package: libydb-cpp-otel-metrics-dev
Architecture: any
Depends: \${misc:Depends}, libydb-cpp-dev (= \${binary:Version})
Description: YDB C++ SDK OpenTelemetry metrics plugin development files
 Static library and headers for YDB C++ SDK OpenTelemetry metrics plugin.

Package: libydb-cpp-otel-tracing-dev
Architecture: any
Depends: \${misc:Depends}, libydb-cpp-dev (= \${binary:Version})
Description: YDB C++ SDK OpenTelemetry tracing plugin development files
 Static library and headers for YDB C++ SDK OpenTelemetry tracing plugin.
EOF_CONTROL

cat <<'EOF_RULES' > debian/rules
#!/usr/bin/make -f

export DEB_BUILD_MAINT_OPTIONS = hardening=+all

%:
	dh $@ --buildsystem=cmake

override_dh_auto_configure:
	dh_auto_configure -- \
		-DCMAKE_BUILD_TYPE=Release \
		-DYDB_SDK_INSTALL=ON \
		-DYDB_SDK_EXAMPLES=OFF \
		-DYDB_SDK_TESTS=OFF \
		-DYDB_SDK_ENABLE_OTEL_METRICS=ON \
		-DYDB_SDK_ENABLE_OTEL_TRACE=ON \
		-DBUILD_SHARED_LIBS=OFF \
		-DYDB_SDK_USE_SYSTEM_GOOGLEAPIS=ON \
		-DCMAKE_INSTALL_PREFIX=/usr/share/yandex \
		-DCMAKE_PREFIX_PATH="/usr/share/yandex"
EOF_RULES
chmod +x debian/rules

cat <<EOF_INSTALL > debian/libydb-cpp-dev.install
debian/tmp/usr/share/yandex/lib/*/libydb-cpp.a usr/share/yandex/lib/
debian/tmp/usr/share/yandex/include/ydb-cpp-sdk usr/share/yandex/include/
debian/tmp/usr/share/yandex/include/__ydb_sdk_special_headers usr/share/yandex/include/
debian/tmp/usr/share/yandex/lib/*/cmake/ydb-cpp-sdk usr/share/yandex/lib/*/cmake/
debian/tmp/usr/share/yandex/include/libbase64.h usr/share/yandex/include/
debian/tmp/usr/share/yandex/lib/*/libbase64.a usr/share/yandex/lib/
debian/tmp/usr/share/yandex/lib/*/cmake/base64 usr/share/yandex/lib/*/cmake/
debian/tmp/usr/share/yandex/include/picojson usr/share/yandex/include/
debian/tmp/usr/share/yandex/include/jwt-cpp usr/share/yandex/include/
debian/tmp/usr/share/yandex/cmake/jwt-cpp* usr/share/yandex/cmake/
debian/tmp/usr/share/yandex/include/opentelemetry usr/share/yandex/include/
debian/tmp/usr/share/yandex/lib/*/libopentelemetry_* usr/share/yandex/lib/
debian/tmp/usr/share/yandex/lib/*/cmake/opentelemetry-cpp usr/share/yandex/lib/*/cmake/
debian/tmp/usr/share/yandex/lib/*/pkgconfig/opentelemetry_*.pc usr/share/yandex/lib/*/pkgconfig/
EOF_INSTALL

cat <<EOF_INSTALL > debian/libydb-cpp-iam-dev.install
debian/tmp/usr/share/yandex/lib/*/libydb-cpp-iam.a usr/share/yandex/lib/
EOF_INSTALL

cat <<EOF_INSTALL > debian/libydb-cpp-otel-metrics-dev.install
debian/tmp/usr/share/yandex/lib/*/libydb-cpp-otel-metrics.a usr/share/yandex/lib/
debian/tmp/usr/share/yandex/include/ydb-cpp-sdk/open_telemetry/metrics.h usr/share/yandex/include/ydb-cpp-sdk/open_telemetry/
EOF_INSTALL

cat <<EOF_INSTALL > debian/libydb-cpp-otel-tracing-dev.install
debian/tmp/usr/share/yandex/lib/*/libydb-cpp-otel-tracing.a usr/share/yandex/lib/
debian/tmp/usr/share/yandex/include/ydb-cpp-sdk/open_telemetry/trace.h usr/share/yandex/include/ydb-cpp-sdk/open_telemetry/
EOF_INSTALL


mkdir -p debian/source
echo "3.0 (quilt)" > debian/source/format

echo "Generated debian directory for version ${DEB_VERSION}"
