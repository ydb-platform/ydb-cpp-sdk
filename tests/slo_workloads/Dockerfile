# syntax=docker/dockerfile:1.7
FROM ubuntu:24.04 AS sdk

ENV DEBIAN_FRONTEND=noninteractive \
    CPM_SOURCE_CACHE=/root/.cache/cpm/sources \
    CCACHE_DIR=/root/.ccache \
    CCACHE_MAXSIZE=2G \
    YDB_CPP_SDK_INSTALL_PREFIX=/opt/ydb-cpp-sdk

RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates ccache clang cmake git libidn11-dev libssl-dev ninja-build \
      pkg-config python3 ragel yasm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ydb-cpp-sdk

# Keep the expensive SDK install independent from the workload harness. Most
# SLO changes touch tests/slo_workloads only and should reuse this whole layer.
COPY CMakeLists.txt LICENSE ./
COPY cmake/ cmake/
COPY contrib/ contrib/
COPY include/ include/
COPY library/ library/
COPY plugins/ plugins/
COPY scripts/ scripts/
COPY src/ src/
COPY third_party/ third_party/
COPY tools/ tools/
COPY util/ util/

# CPM provides every non-platform library during this single SDK build.
RUN --mount=type=cache,target=/root/.cache/cpm,sharing=locked \
    --mount=type=cache,target=/root/.ccache,sharing=locked \
    cmake -E remove_directory /root/.cache/cpm/build-sdk \
    && cmake -S . -B /root/.cache/cpm/build-sdk -G Ninja \
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
    && cmake --build /root/.cache/cpm/build-sdk --parallel \
    && cmake --install /root/.cache/cpm/build-sdk

FROM sdk AS workload

COPY tests/slo_workloads/ tests/slo_workloads/

ARG SLO_TARGET=slo-key-value
ARG SLO_BINARY=tests/slo_workloads/key_value/slo-key-value
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
    && cmake --build build --target ${SLO_TARGET} --parallel \
    && cmake -E create_symlink "/ydb-cpp-sdk/build/${SLO_BINARY}" /usr/local/bin/slo-workload

ENTRYPOINT ["/usr/local/bin/slo-workload"]
