# syntax=docker/dockerfile:1.7
FROM ubuntu:24.04

ARG PRESET=release-test-clang
ENV DEBIAN_FRONTEND=noninteractive \
    CPM_SOURCE_CACHE=/root/.cache/cpm \
    CCACHE_DIR=/root/.ccache \
    CCACHE_MAXSIZE=2G \
    YDB_CPP_SDK_INSTALL_PREFIX=/opt/ydb-cpp-sdk

RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates ccache clang cmake git libidn11-dev libssl-dev ninja-build \
      pkg-config python3 ragel yasm \
    && rm -rf /var/lib/apt/lists/*

COPY . /ydb-cpp-sdk
WORKDIR /ydb-cpp-sdk

# CPM provides every non-platform library during this single SDK build.
RUN --mount=type=cache,target=/root/.cache/cpm,sharing=locked \
    --mount=type=cache,target=/root/.ccache,sharing=locked \
    cmake -S . -B build-sdk -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_C_COMPILER_LAUNCHER=ccache \
      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
      -DCMAKE_INSTALL_PREFIX=${YDB_CPP_SDK_INSTALL_PREFIX} \
      -DYDB_SDK_DEPENDENCY_MODE=CPM \
      -DYDB_SDK_INSTALL=ON \
      -DYDB_SDK_EXAMPLES=OFF \
      -DYDB_SDK_TESTS=OFF \
      -DYDB_SDK_ENABLE_OTEL_METRICS=ON \
    && cmake --build build-sdk --parallel \
    && cmake --install build-sdk

RUN --mount=type=cache,target=/root/.cache/cpm,sharing=locked \
    --mount=type=cache,target=/root/.ccache,sharing=locked \
    cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_C_COMPILER_LAUNCHER=ccache \
      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
      -DCMAKE_PREFIX_PATH=${YDB_CPP_SDK_INSTALL_PREFIX} \
      -DYDB_CPP_SDK_SLO_USE_INSTALLED_SDK=ON \
    && cmake --build build --target slo-key-value --parallel

ENTRYPOINT ["./build/tests/slo_workloads/key_value/slo-key-value"]
