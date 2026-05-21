#!/bin/bash
# Generate debian/ directory for yandex-googleapis-api-common-protos source package.
# This script is intended to be run from the repository root or from
# scripts/googleapis_deb/ itself.  It produces a debian/ directory inside
# scripts/googleapis_deb/ that is suitable for dpkg-buildpackage / debuild.
set -euo pipefail

SCRIPT_DIR=$(dirname "$(realpath "$0")")
REPO_ROOT=$(realpath "$SCRIPT_DIR/../..")

SERIES="${1:-noble}"

# ---------------------------------------------------------------------------
# Version
# ---------------------------------------------------------------------------
VERSION=$(grep -oP 'CPACK_PACKAGE_VERSION\s+"\K[^"]+' "$SCRIPT_DIR/CMakeLists.txt" || echo "1.0.0")
GIT_COMMIT=$(cd "$REPO_ROOT" && git rev-parse --short HEAD 2>/dev/null || echo "unknown")
DEB_VERSION="${VERSION}-1~${SERIES}1"

# ---------------------------------------------------------------------------
# debian/ skeleton
# ---------------------------------------------------------------------------
DEB_DIR="$SCRIPT_DIR/debian"
mkdir -p "$DEB_DIR/source"

# -- changelog --------------------------------------------------------------
cat > "$DEB_DIR/changelog" <<EOF
yandex-googleapis-api-common-protos (${DEB_VERSION}) ${SERIES}; urgency=low

  * Automated package build
  * Git commit: ${GIT_COMMIT}

 -- YDB Team <ydb-team@ydb.tech>  $(date -R)
EOF

# -- control ----------------------------------------------------------------
cat > "$DEB_DIR/control" <<'EOF'
Source: yandex-googleapis-api-common-protos
Section: libdevel
Priority: optional
Maintainer: YDB Team <ydb-team@ydb.tech>
Build-Depends: debhelper-compat (= 13),
 cmake,
 protobuf-compiler,
 libprotobuf-dev
Standards-Version: 4.6.2
Homepage: https://github.com/googleapis/api-common-protos
Rules-Requires-Root: no

Package: yandex-googleapis-api-common-protos
Architecture: any
Depends: ${misc:Depends}, libprotobuf-dev
Description: Google API common proto compiled libraries (Yandex build)
 Pre-compiled C++ static libraries and headers generated from the
 googleapis/api-common-protos proto definitions.  Installed under
 /usr/share/yandex so that YDB C++ SDK and other Yandex packages can
 find_package(yandex-googleapis-api-common-protos).
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
		-DCMAKE_INSTALL_PREFIX=/usr/share/yandex

override_dh_auto_install:
	dh_auto_install --destdir=debian/tmp
RULES
chmod +x "$DEB_DIR/rules"

# -- install ----------------------------------------------------------------
cat > "$DEB_DIR/yandex-googleapis-api-common-protos.install" <<'EOF'
debian/tmp/usr/share/yandex/lib/*/libapi-common-protos.a usr/share/yandex/lib/
debian/tmp/usr/share/yandex/include/google usr/share/yandex/include/
debian/tmp/usr/share/yandex/lib/*/cmake/yandex-googleapis-api-common-protos usr/share/yandex/lib/cmake/
EOF

# -- source/format ----------------------------------------------------------
echo "3.0 (quilt)" > "$DEB_DIR/source/format"

echo "Generated debian directory for yandex-googleapis-api-common-protos ${DEB_VERSION} (series: ${SERIES})"
