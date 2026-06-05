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

COPY test_project /test_project
WORKDIR /test_project