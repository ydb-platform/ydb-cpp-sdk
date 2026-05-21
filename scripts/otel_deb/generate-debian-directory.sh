#!/bin/bash
# Generate debian/ directory for yandex-opentelemetry-cpp source package.
# This wraps the upstream opentelemetry-cpp v1.12.0 into a Debian source
# package that installs under /usr/share/yandex so it can be consumed by
# the YDB C++ SDK packaging build.
set -euo pipefail

SCRIPT_DIR=$(dirname "$(realpath "$0")")

SERIES="${1:-noble}"
OTEL_VERSION="1.12.0"
GIT_COMMIT="v${OTEL_VERSION}"
DEB_VERSION="${OTEL_VERSION}-1~${SERIES}1"

# ---------------------------------------------------------------------------
# debian/ skeleton
# ---------------------------------------------------------------------------
DEB_DIR="$SCRIPT_DIR/debian"
mkdir -p "$DEB_DIR/source"

# -- changelog --------------------------------------------------------------
cat > "$DEB_DIR/changelog" <<EOF
yandex-opentelemetry-cpp (${DEB_VERSION}) ${SERIES}; urgency=low

  * Automated package build of opentelemetry-cpp ${OTEL_VERSION}
  * Upstream tag: ${GIT_COMMIT}

 -- YDB Team <ydb-team@ydb.tech>  $(date -R)
EOF

# -- control ----------------------------------------------------------------
cat > "$DEB_DIR/control" <<'EOF'
Source: yandex-opentelemetry-cpp
Section: libdevel
Priority: optional
Maintainer: YDB Team <ydb-team@ydb.tech>
Build-Depends: debhelper-compat (= 13),
 cmake,
 git,
 pkg-config,
 libcurl4-openssl-dev,
 nlohmann-json3-dev
Standards-Version: 4.6.2
Homepage: https://opentelemetry.io/docs/languages/cpp/
Rules-Requires-Root: no

Package: yandex-opentelemetry-cpp-dev
Architecture: any
Depends: ${misc:Depends}
Description: OpenTelemetry C++ SDK (Yandex build)
 Static libraries, headers and CMake package files for the OpenTelemetry
 C++ SDK.  Installed under /usr/share/yandex so that YDB C++ SDK and
 other Yandex packages can find_package(opentelemetry-cpp).
EOF

# -- rules ------------------------------------------------------------------
cat > "$DEB_DIR/rules" <<'RULES'
#!/usr/bin/make -f

export DEB_BUILD_MAINT_OPTIONS = hardening=+all

%:
	dh $@ --buildsystem=cmake

override_dh_auto_configure:
	dh_auto_configure -- \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr/share/yandex \
		-DFETCHCONTENT_FULLY_DISCONNECTED=OFF

override_dh_auto_install:
	dh_auto_install --destdir=debian/tmp
RULES
chmod +x "$DEB_DIR/rules"

# -- install ----------------------------------------------------------------
cat > "$DEB_DIR/yandex-opentelemetry-cpp-dev.install" <<'EOF'
debian/tmp/usr/share/yandex/lib/*/libopentelemetry_*.a usr/share/yandex/lib/
debian/tmp/usr/share/yandex/include/opentelemetry usr/share/yandex/include/
debian/tmp/usr/share/yandex/lib/*/cmake/opentelemetry-cpp usr/share/yandex/lib/cmake/
debian/tmp/usr/share/yandex/lib/*/pkgconfig/opentelemetry_*.pc usr/share/yandex/lib/pkgconfig/
EOF

# -- source/format ----------------------------------------------------------
echo "3.0 (quilt)" > "$DEB_DIR/source/format"

echo "Generated debian directory for yandex-opentelemetry-cpp ${DEB_VERSION} (series: ${SERIES})"
