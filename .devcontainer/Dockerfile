FROM --platform=linux/amd64 mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

# Install software-properties-common for add-apt-repository
RUN apt-get update && apt-get -y install software-properties-common

# Install CMake
ENV CMAKE_VERSION=3.27.7
RUN add-apt-repository ppa:ubuntu-toolchain-r/test && apt-get -y update && apt-get -y install make && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz && \
    tar -xvzf cmake-${CMAKE_VERSION}.tar.gz && cd cmake-${CMAKE_VERSION} && \
    ./bootstrap && \
    make -j$(nproc) && \
    make install

# Install C++ tools and libraries
RUN apt-get -y update && apt-get -y install \
    git gdb ninja-build libidn11-dev ragel yasm protobuf-compiler \
    protobuf-compiler-grpc libprotobuf-dev libgrpc++-dev libgrpc-dev libgrpc++1 libgrpc10 \
    rapidjson-dev zlib1g-dev libxxhash-dev libzstd-dev libsnappy-dev libgtest-dev libgmock-dev \
    libbz2-dev libdouble-conversion-dev libstdc++-13-dev gcc-13 g++-13 liblz4-dev libssl-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install LLVM
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod u+x llvm.sh && \
    ./llvm.sh 16

# Update alternatives to use clang-16 by default
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-16 10000 && \
    update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-16 10000 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-16 10000

# Update alternatives to use gcc-13 by default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 10000 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 10000

# Install libiconv
ENV LIBICONV_VERSION=1.15
RUN wget https://ftp.gnu.org/pub/gnu/libiconv/libiconv-${LIBICONV_VERSION}.tar.gz && \
    tar -xvzf libiconv-${LIBICONV_VERSION}.tar.gz && cd libiconv-${LIBICONV_VERSION} && \
    ./configure --prefix=/usr/local && \
    make && \
    make install

# Install base64
ENV BASE64_VERSION=0.5.2
RUN wget -O base64-${BASE64_VERSION}.tar.gz https://github.com/aklomp/base64/archive/refs/tags/v${BASE64_VERSION}.tar.gz && \
    tar -xvzf base64-${BASE64_VERSION}.tar.gz && cd base64-${BASE64_VERSION} && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --config Release --target install

# Install brotli
ENV BROTLI_VERSION=1.1.0
RUN wget -O brotli-${BROTLI_VERSION}.tar.gz https://github.com/google/brotli/archive/refs/tags/v${BROTLI_VERSION}.tar.gz && \
    tar -xvzf brotli-${BROTLI_VERSION}.tar.gz && cd brotli-${BROTLI_VERSION} && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --config Release --target install

# Install jwt-cpp
ENV JWT_CPP_VERSION=0.7.0
RUN wget -O jwt-cpp-${JWT_CPP_VERSION}.tar.gz https://github.com/Thalhammer/jwt-cpp/archive/refs/tags/v${JWT_CPP_VERSION}.tar.gz && \
    tar -xvzf jwt-cpp-${JWT_CPP_VERSION}.tar.gz && cd jwt-cpp-${JWT_CPP_VERSION} && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --config Release --target install

# Install ccache 4.8.1 or above
ENV CCACHE_VERSION=4.8.1
RUN wget https://github.com/ccache/ccache/releases/download/v${CCACHE_VERSION}/ccache-${CCACHE_VERSION}-linux-x86_64.tar.xz \
    && tar -xf ccache-${CCACHE_VERSION}-linux-x86_64.tar.xz \
    && cp ccache-${CCACHE_VERSION}-linux-x86_64/ccache /usr/local/bin/ \
    && rm -rf ccache-${CCACHE_VERSION}-linux-x86_64 ccache-${CCACHE_VERSION}-linux-x86_64.tar.xz

# Install ydb cli 2.16.0
ENV YDB_VERSION=2.16.0
RUN wget https://storage.yandexcloud.net/yandexcloud-ydb/release/${YDB_VERSION}/linux/amd64/ydb && \
    chmod u+x ydb && \
    mv ydb /usr/local/bin/
