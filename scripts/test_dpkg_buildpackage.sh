#!/bin/bash
set -e

SCRIPT_DIR=$(dirname "$(realpath "$0")")
SOURCE_DIR=$(realpath "$SCRIPT_DIR/..")

echo "Building test Docker image for dpkg-buildpackage..."

# We need a Dockerfile that has all the build dependencies for dpkg-buildpackage
DOCKERFILE=$(mktemp)
cat <<EOF > "$DOCKERFILE"
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \\
    build-essential \\
    cmake \\
    pkg-config \\
    git \\
    libidn11-dev \\
    libssl-dev \\
    zlib1g-dev \\
    libprotobuf-dev \\
    protobuf-compiler \\
    libgrpc++-dev \\
    protobuf-compiler-grpc \\
    libbrotli-dev \\
    liblz4-dev \\
    libzstd-dev \\
    libbz2-dev \\
    libxxhash-dev \\
    libsnappy-dev \\
    libdouble-conversion-dev \\
    libgtest-dev \\
    libre2-dev \\
    libc-ares-dev \\
    rapidjson-dev \\
    debhelper \\
    dpkg-dev \\
    python3 \\
    python3-six \\
    ragel \\
    yasm \\
    && rm -rf /var/lib/apt/lists/*
WORKDIR /source
EOF

docker build -t ydb-cpp-sdk-dpkg-test -f "$DOCKERFILE" .
rm -f "$DOCKERFILE"

# Prepare a clean tarball or just copy the directory inside the container
echo "Running dpkg-buildpackage in the container..."
docker run --rm \
    -v "$SOURCE_DIR:/source:ro" \
    ydb-cpp-sdk-dpkg-test \
    bash -c "cp -r /source /tmp/source && cd /tmp/source && \
             rm -rf build_googleapis_deb && \
             cmake -S scripts/googleapis_deb -B build_googleapis_deb -DCMAKE_INSTALL_PREFIX=/usr/share/yandex && \
             cmake --build build_googleapis_deb -j\$(nproc) && \
             cmake --build build_googleapis_deb --target package && \
             dpkg -i build_googleapis_deb/*.deb && \
             ./scripts/generate-debian-directory.sh && \
             dpkg-buildpackage -us -uc -b -d -j\$(nproc)"

echo "Test successful! dpkg-buildpackage works."