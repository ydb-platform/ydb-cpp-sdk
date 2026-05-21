#!/bin/bash
# =============================================================================
# upload_to_ppa.sh — Build Debian source packages and upload them to a
# Launchpad PPA.
#
# Usage:
#   ./scripts/upload_to_ppa.sh [OPTIONS]
#
# Options:
#   --ppa PPA             PPA identifier (default: ppa:ydb-team/ydb-cpp-sdk)
#   --series SERIES       Ubuntu series name (default: noble)
#   --googleapis          Build/upload only the googleapis package
#   --otel                Build/upload only the opentelemetry-cpp package
#   --sdk                 Build/upload only the ydb-cpp-sdk packages
#   --all                 Build/upload all packages (default)
#   --gpg-key KEY_ID      GPG key fingerprint for signing
#   --dry-run             Build source packages but do not upload
#   --skip-orig           Pass -sd to debuild (reuse existing .orig.tar.gz)
#   --help                Show this help message
# =============================================================================
set -euo pipefail

SCRIPT_DIR=$(dirname "$(realpath "$0")")
REPO_ROOT=$(realpath "$SCRIPT_DIR/..")

# ---- defaults ---------------------------------------------------------------
PPA="ppa:ydb-team/ydb-cpp-sdk"
SERIES="noble"
GPG_KEY=""
DRY_RUN=false
SKIP_ORIG=false
BUILD_GOOGLEAPIS=false
BUILD_OTEL=false
BUILD_SDK=false
BUILD_ALL=true

# ---- parse arguments --------------------------------------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        --ppa)        PPA="$2";       shift 2 ;;
        --ppa=*)      PPA="${1#*=}";   shift   ;;
        --series)     SERIES="$2";    shift 2 ;;
        --series=*)   SERIES="${1#*=}"; shift  ;;
        --gpg-key)    GPG_KEY="$2";   shift 2 ;;
        --gpg-key=*)  GPG_KEY="${1#*=}"; shift ;;
        --googleapis) BUILD_GOOGLEAPIS=true; BUILD_ALL=false; shift ;;
        --otel)       BUILD_OTEL=true;       BUILD_ALL=false; shift ;;
        --sdk)        BUILD_SDK=true;        BUILD_ALL=false; shift ;;
        --all)        BUILD_ALL=true;        shift ;;
        --dry-run)    DRY_RUN=true;          shift ;;
        --skip-orig)  SKIP_ORIG=true;        shift ;;
        --help)
            head -n 20 "$0" | grep '^#' | sed 's/^# \?//'
            exit 0
            ;;
        *)
            echo "Error: unknown option '$1'" >&2
            echo "Run $0 --help for usage." >&2
            exit 1
            ;;
    esac
done

if $BUILD_ALL; then
    BUILD_GOOGLEAPIS=true
    BUILD_OTEL=true
    BUILD_SDK=true
fi

# ---- prerequisites ----------------------------------------------------------
for cmd in debuild dput gpg git tar; do
    if ! command -v "$cmd" &>/dev/null; then
        echo "Error: '$cmd' is not installed. Install it first:" >&2
        echo "  sudo apt install devscripts dput gnupg git" >&2
        exit 1
    fi
done

DEBUILD_SIGN_ARGS=()
if [[ -n "$GPG_KEY" ]]; then
    DEBUILD_SIGN_ARGS=(-k"$GPG_KEY")
fi

DEBUILD_ORIG_FLAG="-sa"
if $SKIP_ORIG; then
    DEBUILD_ORIG_FLAG="-sd"
fi

# ---- helper: upload or dry-run ----------------------------------------------
do_upload() {
    local changes_file="$1"
    if $DRY_RUN; then
        echo "[dry-run] Would upload: $changes_file"
        echo "[dry-run]   dput $PPA $changes_file"
    else
        echo "Uploading $changes_file to $PPA ..."
        dput "$PPA" "$changes_file"
    fi
}

# =============================================================================
# 1. yandex-googleapis-api-common-protos
# =============================================================================
if $BUILD_GOOGLEAPIS; then
    echo "============================================================"
    echo "Building source package: yandex-googleapis-api-common-protos"
    echo "============================================================"

    GAPI_SRC="$SCRIPT_DIR/googleapis_deb"
    GAPI_VERSION=$(grep -oP 'CPACK_PACKAGE_VERSION\s+"\K[^"]+' "$GAPI_SRC/CMakeLists.txt" || echo "1.0.0")
    GAPI_DEB_VERSION="${GAPI_VERSION}-1~${SERIES}1"
    GAPI_PKG="yandex-googleapis-api-common-protos"

    WORK_DIR=$(mktemp -d)
    trap "rm -rf $WORK_DIR" EXIT

    GAPI_BUILD_DIR="$WORK_DIR/${GAPI_PKG}-${GAPI_VERSION}"
    mkdir -p "$GAPI_BUILD_DIR"

    # Copy CMakeLists.txt and config template
    cp "$GAPI_SRC/CMakeLists.txt" "$GAPI_BUILD_DIR/"
    cp "$GAPI_SRC/yandex-googleapis-api-common-protosConfig.cmake.in" "$GAPI_BUILD_DIR/"

    # Copy proto sources (the submodule content)
    if [[ -d "$REPO_ROOT/third_party/api-common-protos/google" ]]; then
        mkdir -p "$GAPI_BUILD_DIR/third_party/api-common-protos"
        cp -r "$REPO_ROOT/third_party/api-common-protos/google" \
              "$GAPI_BUILD_DIR/third_party/api-common-protos/"
    else
        echo "Error: third_party/api-common-protos/google not found." >&2
        echo "Initialize git submodules: git submodule update --init" >&2
        exit 1
    fi

    # Fix CMakeLists.txt PROTOS_DIR to point to local copy
    sed -i 's|"\${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/api-common-protos"|"${CMAKE_CURRENT_SOURCE_DIR}/third_party/api-common-protos"|' \
        "$GAPI_BUILD_DIR/CMakeLists.txt"

    # Generate debian/ directory
    bash "$GAPI_SRC/generate-debian-directory.sh" "$SERIES"
    cp -r "$GAPI_SRC/debian" "$GAPI_BUILD_DIR/"

    # Create .orig.tar.gz
    ORIG_TAR="$WORK_DIR/${GAPI_PKG}_${GAPI_VERSION}.orig.tar.gz"
    tar -czf "$ORIG_TAR" -C "$WORK_DIR" "${GAPI_PKG}-${GAPI_VERSION}"

    # Build source package
    cd "$GAPI_BUILD_DIR"
    debuild -S "$DEBUILD_ORIG_FLAG" "${DEBUILD_SIGN_ARGS[@]}"

    # Upload
    CHANGES_FILE="$WORK_DIR/${GAPI_PKG}_${GAPI_DEB_VERSION}_source.changes"
    if [[ -f "$CHANGES_FILE" ]]; then
        do_upload "$CHANGES_FILE"
    else
        echo "Warning: expected changes file not found: $CHANGES_FILE" >&2
        # Try to find it
        CHANGES_FILE=$(ls "$WORK_DIR"/${GAPI_PKG}_*_source.changes 2>/dev/null | head -1)
        if [[ -n "$CHANGES_FILE" ]]; then
            do_upload "$CHANGES_FILE"
        else
            echo "Error: no _source.changes file found for googleapis." >&2
            exit 1
        fi
    fi

    cd "$REPO_ROOT"
    echo "googleapis source package built successfully."
    echo ""
fi

# =============================================================================
# 2. yandex-opentelemetry-cpp
# =============================================================================
if $BUILD_OTEL; then
    echo "============================================================"
    echo "Building source package: yandex-opentelemetry-cpp"
    echo "============================================================"

    OTEL_SRC="$SCRIPT_DIR/otel_deb"
    OTEL_VERSION="1.12.0"
    OTEL_DEB_VERSION="${OTEL_VERSION}-1~${SERIES}1"
    OTEL_PKG="yandex-opentelemetry-cpp"

    WORK_DIR_OTEL=$(mktemp -d)
    trap "rm -rf $WORK_DIR_OTEL" EXIT

    OTEL_BUILD_DIR="$WORK_DIR_OTEL/${OTEL_PKG}-${OTEL_VERSION}"
    mkdir -p "$OTEL_BUILD_DIR"

    # Copy CMakeLists.txt
    cp "$OTEL_SRC/CMakeLists.txt" "$OTEL_BUILD_DIR/"

    # Generate debian/ directory
    bash "$OTEL_SRC/generate-debian-directory.sh" "$SERIES"
    cp -r "$OTEL_SRC/debian" "$OTEL_BUILD_DIR/"

    # Create .orig.tar.gz
    ORIG_TAR_OTEL="$WORK_DIR_OTEL/${OTEL_PKG}_${OTEL_VERSION}.orig.tar.gz"
    tar -czf "$ORIG_TAR_OTEL" -C "$WORK_DIR_OTEL" "${OTEL_PKG}-${OTEL_VERSION}"

    # Build source package
    cd "$OTEL_BUILD_DIR"
    debuild -S "$DEBUILD_ORIG_FLAG" "${DEBUILD_SIGN_ARGS[@]}"

    # Upload
    CHANGES_FILE_OTEL="$WORK_DIR_OTEL/${OTEL_PKG}_${OTEL_DEB_VERSION}_source.changes"
    if [[ -f "$CHANGES_FILE_OTEL" ]]; then
        do_upload "$CHANGES_FILE_OTEL"
    else
        CHANGES_FILE_OTEL=$(ls "$WORK_DIR_OTEL"/${OTEL_PKG}_*_source.changes 2>/dev/null | head -1)
        if [[ -n "$CHANGES_FILE_OTEL" ]]; then
            do_upload "$CHANGES_FILE_OTEL"
        else
            echo "Error: no _source.changes file found for opentelemetry-cpp." >&2
            exit 1
        fi
    fi

    cd "$REPO_ROOT"
    echo "opentelemetry-cpp source package built successfully."
    echo ""
fi

# =============================================================================
# 3. ydb-cpp-sdk
# =============================================================================
if $BUILD_SDK; then
    echo "============================================================"
    echo "Building source package: ydb-cpp-sdk"
    echo "============================================================"

    SDK_VERSION=$(grep -E 'YDB_SDK_VERSION = "[0-9]+\.[0-9]+\.[0-9]+"' "$REPO_ROOT/src/version.h" \
        | sed -E 's/.*"([0-9]+\.[0-9]+\.[0-9]+)".*/\1/')
    if [[ -z "$SDK_VERSION" ]]; then
        echo "Error: Could not extract YDB_SDK_VERSION from src/version.h" >&2
        exit 1
    fi
    SDK_DEB_VERSION="${SDK_VERSION}-1~${SERIES}1"
    SDK_PKG="ydb-cpp-sdk"

    WORK_DIR_SDK=$(mktemp -d)
    trap "rm -rf $WORK_DIR_SDK" EXIT

    SDK_BUILD_DIR="$WORK_DIR_SDK/${SDK_PKG}-${SDK_VERSION}"

    # Create source tarball from git archive (includes submodules content)
    echo "Creating source tarball..."
    cd "$REPO_ROOT"
    git archive --format=tar --prefix="${SDK_PKG}-${SDK_VERSION}/" HEAD \
        | tar -xf - -C "$WORK_DIR_SDK"

    # Copy submodule contents (git archive doesn't include them)
    if [[ -d "$REPO_ROOT/third_party/api-common-protos/google" ]]; then
        mkdir -p "$SDK_BUILD_DIR/third_party/api-common-protos"
        cp -r "$REPO_ROOT/third_party/api-common-protos/google" \
              "$SDK_BUILD_DIR/third_party/api-common-protos/"
    fi
    if [[ -d "$REPO_ROOT/third_party/FastLZ" ]]; then
        mkdir -p "$SDK_BUILD_DIR/third_party/FastLZ"
        cp -r "$REPO_ROOT/third_party/FastLZ/"* "$SDK_BUILD_DIR/third_party/FastLZ/" 2>/dev/null || true
    fi

    # Generate debian/ directory
    bash "$SCRIPT_DIR/generate-debian-directory.sh" --series "$SERIES"
    cp -r "$REPO_ROOT/debian" "$SDK_BUILD_DIR/"

    # Create .orig.tar.gz
    ORIG_TAR_SDK="$WORK_DIR_SDK/${SDK_PKG}_${SDK_VERSION}.orig.tar.gz"
    tar -czf "$ORIG_TAR_SDK" -C "$WORK_DIR_SDK" "${SDK_PKG}-${SDK_VERSION}"

    # Build source package
    cd "$SDK_BUILD_DIR"
    debuild -S "$DEBUILD_ORIG_FLAG" "${DEBUILD_SIGN_ARGS[@]}"

    # Upload
    CHANGES_FILE_SDK="$WORK_DIR_SDK/${SDK_PKG}_${SDK_DEB_VERSION}_source.changes"
    if [[ -f "$CHANGES_FILE_SDK" ]]; then
        do_upload "$CHANGES_FILE_SDK"
    else
        CHANGES_FILE_SDK=$(ls "$WORK_DIR_SDK"/${SDK_PKG}_*_source.changes 2>/dev/null | head -1)
        if [[ -n "$CHANGES_FILE_SDK" ]]; then
            do_upload "$CHANGES_FILE_SDK"
        else
            echo "Error: no _source.changes file found for ydb-cpp-sdk." >&2
            exit 1
        fi
    fi

    cd "$REPO_ROOT"
    echo "ydb-cpp-sdk source package built successfully."
    echo ""
fi

echo "============================================================"
echo "Done."
if $DRY_RUN; then
    echo "(dry-run mode — nothing was uploaded)"
fi
echo "============================================================"
