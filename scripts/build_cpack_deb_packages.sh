#!/usr/bin/env bash
set -euo pipefail

SOURCE_DIR="${SOURCE_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
OUTPUT_DIR="${1:-${SOURCE_DIR}/build-deb/packages}"
BUILD_ROOT="${YDB_DEB_BUILD_ROOT:-${SOURCE_DIR}/build-deb}"
CPM_SOURCE_CACHE="${CPM_SOURCE_CACHE:-${SOURCE_DIR}/.cache/cpm}"
CCACHE_DIR="${CCACHE_DIR:-${SOURCE_DIR}/.cache/ccache}"

mkdir -p "$OUTPUT_DIR" "$BUILD_ROOT" "$CPM_SOURCE_CACHE" "$CCACHE_DIR"
OUTPUT_DIR="$(realpath "$OUTPUT_DIR")"
BUILD_ROOT="$(realpath "$BUILD_ROOT")"

if [ "${EUID}" -eq 0 ]; then
    SUDO=()
else
    SUDO=(sudo)
fi

if [ "${YDB_DEB_INSTALL_DEPS:-1}" = "1" ]; then
    export DEBIAN_FRONTEND=noninteractive
    "${SUDO[@]}" apt-get update
    "${SUDO[@]}" apt-get install -y --no-install-recommends \
        build-essential ca-certificates ccache cmake ninja-build pkg-config git \
        libidn11-dev libssl-dev zlib1g-dev \
        libprotobuf-dev protobuf-compiler libgrpc++-dev protobuf-compiler-grpc \
        libbrotli-dev liblz4-dev libzstd-dev libbz2-dev libxxhash-dev \
        libsnappy-dev libdouble-conversion-dev libtbb-dev libre2-dev \
        libc-ares-dev rapidjson-dev python3 ragel yasm
fi

export CPM_SOURCE_CACHE CCACHE_DIR
compiler_cache_args=()
if command -v ccache >/dev/null 2>&1; then
    compiler_cache_args=(
        -DCMAKE_C_COMPILER_LAUNCHER=ccache
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    )
fi

google_build="${BUILD_ROOT}/googleapis"
sdk_build="${BUILD_ROOT}/sdk"

cmake -S "${SOURCE_DIR}/scripts/googleapis_deb" -B "$google_build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCPM_SOURCE_CACHE="$CPM_SOURCE_CACHE" \
    -DFETCHCONTENT_FULLY_DISCONNECTED="${YDB_DEB_FULLY_DISCONNECTED:-OFF}" \
    -DCMAKE_INSTALL_PREFIX=/usr/share/yandex \
    "${compiler_cache_args[@]}"
cmake --build "$google_build" --target package --parallel
"${SUDO[@]}" dpkg -i "$google_build"/*.deb

cmake -S "$SOURCE_DIR" -B "$sdk_build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/share/yandex \
    -DCMAKE_PREFIX_PATH=/usr/share/yandex \
    -DCPM_SOURCE_CACHE="$CPM_SOURCE_CACHE" \
    -DFETCHCONTENT_FULLY_DISCONNECTED="${YDB_DEB_FULLY_DISCONNECTED:-OFF}" \
    -DYDB_SDK_DEPENDENCY_MODE=SYSTEM \
    -DYDB_SDK_INSTALL=ON \
    -DYDB_SDK_EXAMPLES=OFF \
    -DYDB_SDK_TESTS=OFF \
    -DYDB_SDK_ENABLE_OTEL_METRICS=ON \
    -DYDB_SDK_ENABLE_OTEL_TRACE=ON \
    "${compiler_cache_args[@]}"
cmake --build "$sdk_build" --target package --parallel

cp -f "$google_build"/*.deb "$sdk_build"/*.deb "$OUTPUT_DIR"/

if command -v ccache >/dev/null 2>&1; then
    ccache --show-stats || true
fi
ls -la "$OUTPUT_DIR"
