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
    unixodbc \
    odbcinst \
    qt6-base-dev \
    libqt6sql6-odbc \
    && rm -rf /var/lib/apt/lists/*

COPY test_project /test_project
COPY component_test /component_test
COPY odbc_qt_test /odbc_qt_test
RUN cmake -S /odbc_qt_test -B /odbc_qt_test/build \
    && cmake --build /odbc_qt_test/build -j"$(nproc)"
WORKDIR /test_project
