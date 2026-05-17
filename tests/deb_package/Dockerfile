FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
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
    && rm -rf /var/lib/apt/lists/*

RUN git clone -b v1.12.0 https://github.com/open-telemetry/opentelemetry-cpp.git /tmp/otel && \
    cd /tmp/otel && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DWITH_OTLP_HTTP=OFF -DWITH_OTLP_GRPC=OFF -DWITH_PROMETHEUS=OFF -DCMAKE_CXX_STANDARD=20 -DCMAKE_POSITION_INDEPENDENT_CODE=ON && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/otel

COPY test_project /test_project
WORKDIR /test_project