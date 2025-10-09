FROM --platform=linux/amd64 mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

# Install software-properties-common for add-apt-repository
RUN apt-get -y update && apt-get -y install software-properties-common && add-apt-repository ppa:ubuntu-toolchain-r/test

# Install C++ tools and libraries
RUN apt-get -y update && apt-get -y install \
    git gdb ninja-build libidn11-dev ragel yasm libc-ares-dev libre2-dev \
    rapidjson-dev zlib1g-dev libxxhash-dev libzstd-dev libsnappy-dev libgtest-dev libgmock-dev \
    libbz2-dev liblz4-dev libdouble-conversion-dev libssl-dev libstdc++-13-dev gcc-13 g++-13
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install CMake
ENV CMAKE_VERSION=3.27.7
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh \
    -q -O cmake-install.sh \
    && chmod u+x cmake-install.sh \
    && ./cmake-install.sh --skip-license --prefix=/usr/local \
    && rm cmake-install.sh

# Install LLVM
ENV LLVM_VERSION=16
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod u+x llvm.sh && \
    ./llvm.sh ${LLVM_VERSION}

# Update alternatives to use clang-16 by default
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${LLVM_VERSION} 10000 && \
    update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-${LLVM_VERSION} 10000 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${LLVM_VERSION} 10000

# Update alternatives to use gcc-13 by default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 10000 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 10000

# Install abseil-cpp
ENV ABSEIL_CPP_VERSION=20230802.0
ENV ABSEIL_CPP_INSTALL_DIR=~/ydb_deps/abseil-cpp
RUN wget -O abseil-cpp-${ABSEIL_CPP_VERSION}.tar.gz https://github.com/abseil/abseil-cpp/archive/refs/tags/${ABSEIL_CPP_VERSION}.tar.gz && \
    tar -xvzf abseil-cpp-${ABSEIL_CPP_VERSION}.tar.gz && cd abseil-cpp-${ABSEIL_CPP_VERSION} && \
    mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DABSL_PROPAGATE_CXX_STD=ON .. && \
    cmake --build . --config Release && \
    cmake --install . --config Release --prefix ${ABSEIL_CPP_INSTALL_DIR}

# Install protobuf
ENV PROTOBUF_VERSION=3.21.12
ENV PROTOBUF_INSTALL_DIR=~/ydb_deps/protobuf
RUN wget -O protobuf-${PROTOBUF_VERSION}.tar.gz https://github.com/protocolbuffers/protobuf/archive/refs/tags/v${PROTOBUF_VERSION}.tar.gz && \
    tar -xvzf protobuf-${PROTOBUF_VERSION}.tar.gz && cd protobuf-${PROTOBUF_VERSION} && \
    mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_INSTALL=ON -Dprotobuf_ABSL_PROVIDER=package .. && \
    cmake --build . --config Release && \
    cmake --install . --config Release --prefix ${PROTOBUF_INSTALL_DIR}

# Install grpc
ENV GRPC_VERSION=1.54.3
ENV GRPC_INSTALL_DIR=~/ydb_deps/grpc
RUN wget -O grpc-${GRPC_VERSION}.tar.gz https://github.com/grpc/grpc/archive/refs/tags/v${GRPC_VERSION}.tar.gz && \
    tar -xvzf grpc-${GRPC_VERSION}.tar.gz && cd grpc-${GRPC_VERSION} && \
    mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_PREFIX_PATH="${ABSEIL_CPP_INSTALL_DIR};${PROTOBUF_INSTALL_DIR}" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 \
    -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DgRPC_BUILD_CSHARP_EXT=OFF \
    -DgRPC_ZLIB_PROVIDER=package -DgRPC_CARES_PROVIDER=package -DgRPC_RE2_PROVIDER=package \
    -DgRPC_SSL_PROVIDER=package -DgRPC_PROTOBUF_PROVIDER=package -DgRPC_ABSL_PROVIDER=package \
    -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF .. && \
    cmake --build . --config Release && \
    cmake --install . --config Release --prefix ${GRPC_INSTALL_DIR}

# Install base64
ENV BASE64_VERSION=0.5.2
RUN wget -O base64-${BASE64_VERSION}.tar.gz https://github.com/aklomp/base64/archive/refs/tags/v${BASE64_VERSION}.tar.gz && \
    tar -xvzf base64-${BASE64_VERSION}.tar.gz && cd base64-${BASE64_VERSION} && \
    mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --config Release --target install

# Install brotli
ENV BROTLI_VERSION=1.1.0
RUN wget -O brotli-${BROTLI_VERSION}.tar.gz https://github.com/google/brotli/archive/refs/tags/v${BROTLI_VERSION}.tar.gz && \
    tar -xvzf brotli-${BROTLI_VERSION}.tar.gz && cd brotli-${BROTLI_VERSION} && \
    mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --config Release --target install

# Install jwt-cpp
ENV JWT_CPP_VERSION=0.7.0
RUN wget -O jwt-cpp-${JWT_CPP_VERSION}.tar.gz https://github.com/Thalhammer/jwt-cpp/archive/refs/tags/v${JWT_CPP_VERSION}.tar.gz && \
    tar -xvzf jwt-cpp-${JWT_CPP_VERSION}.tar.gz && cd jwt-cpp-${JWT_CPP_VERSION} && \
    mkdir build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
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
