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

CHANGELOG_FILE="debian/changelog"

if [ ! -f "$CHANGELOG_FILE" ]; then
    echo "Error: debian/changelog not found."
    exit 1
fi

DEB_VERSION="${VERSION}-1"

cat <<EOF > "$CHANGELOG_FILE"
ydb-cpp-sdk (${DEB_VERSION}) unstable; urgency=low

  * Automated package build
  * Git commit: ${GIT_COMMIT}

 -- YDB Team <whoami@where>  $(date -R)
EOF

echo "Generated debian directory for version ${DEB_VERSION}"