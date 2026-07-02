#!/bin/bash
set -euo pipefail

SOURCE_DIR="${SOURCE_DIR:-/source}"
if [ ! -d "$SOURCE_DIR" ]; then
    SOURCE_DIR=$(dirname "$(realpath "$0")")/..
    SOURCE_DIR=$(realpath "$SOURCE_DIR")
fi

OUTPUT_DIR="${1:-build-deb}"

cd "$SOURCE_DIR"
mkdir -p "$OUTPUT_DIR"
OUTPUT_DIR=$(realpath "$OUTPUT_DIR")

export DEBIAN_FRONTEND=noninteractive
export CCACHE_DIR="${CCACHE_DIR:-/root/.ccache}"
mkdir -p "$CCACHE_DIR"

if [ "${YDB_DEB_INSTALL_DEPS:-1}" = "1" ]; then
    apt-get update
    apt-get install -y --no-install-recommends \
        build-essential \
        ccache \
        cmake \
        pkg-config \
        git \
        libidn11-dev \
        libssl-dev \
        zlib1g-dev \
        libprotobuf-dev \
        protobuf-compiler \
        libgrpc++-dev \
        protobuf-compiler-grpc \
        libbrotli-dev \
        liblz4-dev \
        libzstd-dev \
        libbz2-dev \
        libxxhash-dev \
        libsnappy-dev \
        libdouble-conversion-dev \
        libgtest-dev \
        libre2-dev \
        libc-ares-dev \
        rapidjson-dev \
        python3 \
        python3-six \
        ragel \
        yasm
fi

touch_existing_sources() {
    for path in "$@"; do
        if [ -e "$path" ]; then
            find "$path" -type f -exec touch {} +
        fi
    done
}

rm -f build_googleapis_deb/*.deb build-deb/*.deb "$OUTPUT_DIR"/*.deb 2>/dev/null || true

if command -v ccache >/dev/null 2>&1; then
    ccache --zero-stats >/dev/null || true
    CMAKE_COMPILER_LAUNCHER_ARGS=(
        -DCMAKE_C_COMPILER_LAUNCHER=ccache
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    )
else
    CMAKE_COMPILER_LAUNCHER_ARGS=()
fi

# Restored build-cache archives can have outputs newer than the checkout.
# Touch package inputs so the incremental build cannot hide source changes.
touch_existing_sources scripts/googleapis_deb third_party/api-common-protos
cmake -S scripts/googleapis_deb -B build_googleapis_deb \
    -DCMAKE_INSTALL_PREFIX=/usr/share/yandex \
    "${CMAKE_COMPILER_LAUNCHER_ARGS[@]}"
cmake --build build_googleapis_deb -j"$(nproc)"
cmake --build build_googleapis_deb --target package
dpkg -i build_googleapis_deb/*.deb

./scripts/generate-debian-directory.sh

touch_existing_sources \
    CMakeLists.txt \
    cmake \
    contrib \
    include \
    library \
    plugins \
    scripts/build_cpack_deb_packages.sh \
    scripts/generate-debian-directory.sh \
    src \
    third_party \
    tools \
    util

cmake -S . -B build-deb \
    -DCMAKE_BUILD_TYPE=Release \
    -DYDB_SDK_INSTALL=ON \
    -DYDB_SDK_EXAMPLES=OFF \
    -DYDB_SDK_TESTS=OFF \
    -DYDB_SDK_ENABLE_OTEL_METRICS=ON \
    -DYDB_SDK_ENABLE_OTEL_TRACE=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DYDB_SDK_USE_SYSTEM_GOOGLEAPIS=ON \
    -DCMAKE_INSTALL_PREFIX=/usr/share/yandex \
    -DCMAKE_PREFIX_PATH="/usr/share/yandex" \
    "${CMAKE_COMPILER_LAUNCHER_ARGS[@]}"
cmake --build build-deb --target package -j"$(nproc)"

cp -f build_googleapis_deb/*.deb "$OUTPUT_DIR"/
if [ "$(realpath build-deb)" != "$OUTPUT_DIR" ]; then
    cp -f build-deb/*.deb "$OUTPUT_DIR"/
fi

if command -v ccache >/dev/null 2>&1; then
    ccache --show-stats || true
fi

chmod -R a+rwX "$CCACHE_DIR" build_googleapis_deb build-deb "$OUTPUT_DIR" debian 2>/dev/null || true
ls -la "$OUTPUT_DIR"
