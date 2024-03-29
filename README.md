# WARNING: THIS IS SUPER MEGA EXTRA ALPHA VERSION OF C++ DRIVER FOR YDB!!!
# Don't use it in production environment!

If you ok with this warning, then...

# Building YDB C++ SDK from sources

## Prerequisites

- cmake 3.22+
- clang 16+
- git 2.20+
- ninja 1.10+
- ragel
- yasm
- protoc

## Library dependencies

- gRPC
- protobuf
- OpenSSL
- Iconv
- IDN
- rapidjson
- xxhash
- zlib
- zstd
- lz4
- snappy 1.1.8+
- base64
- brotli 1.1.10+
- double-conversion
- jwt-cpp

## Runtime requirements

- libidn11-dev (IDN)
- libiconv (Iconv)

## Testing

- gtest
- gmock

## Install dependencies

```bash
sudo apt-get -y update
sudo apt-get -y install git cmake ninja-build libidn11-dev ragel yasm protobuf-compiler \
  protobuf-compiler-grpc libprotobuf-dev libgrpc++-dev libgrpc-dev libgrpc++1 libgrpc10 \
  rapidjson-dev zlib1g-dev libxxhash-dev libzstd-dev libsnappy-dev liblz4-dev \
  libgtest-dev libgmock-dev libbz2-dev libdouble-conversion-dev

wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 16

wget https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz
tar -xvzf libiconv-1.15.tar.gz && cd libiconv-1.15
./configure --prefix=/usr/local
make
sudo make install

wget -O base64-0.5.2.tar.gz https://github.com/aklomp/base64/archive/refs/tags/v0.5.2.tar.gz
tar -xvzf base64-0.5.2.tar.gz && cd base64-0.5.2
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
sudo cmake --build . --config Release --target install

wget -O brotli-1.1.0.tar.gz https://github.com/google/brotli/archive/refs/tags/v1.1.0.tar.gz
tar -xvzf brotli-1.1.0.tar.gz && cd brotli-1.1.0
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
sudo cmake --build . --config Release --target install

wget -O jwt-cpp-0.7.0.tar.gz https://github.com/Thalhammer/jwt-cpp/archive/refs/tags/v0.7.0.tar.gz
tar -xvzf jwt-cpp-0.7.0.tar.gz && cd jwt-cpp-0.7.0
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
sudo cmake --build . --config Release --target install

mkdir -p ~/nayuki_md5
wget -O ~/nayuki_md5/nayuki_md5.c https://www.nayuki.io/res/fast-md5-hash-implementation-in-x86-assembly/md5.c
export NAYUKI_MD5_ROOT=~/nayuki_md5/
```

## Create the work directory

```bash
mkdir ~/ydbwork && cd ~/ydbwork
mkdir build
```

## Install ccache

Install `ccache` into `/usr/local/bin/`. The recommended version is `4.8.1` or above, the minimum required version is `4.7`.

```bash
(V=4.8.1; curl -L https://github.com/ccache/ccache/releases/download/v${V}/ccache-${V}-linux-x86_64.tar.xz | \
sudo tar -xJ -C /usr/local/bin/ --strip-components=1 --no-same-owner ccache-${V}-linux-x86_64/ccache)
```

## Clone the ydb-cpp-sdk repository

```bash
git clone https://github.com/ydb-platform/ydb-cpp-sdk.git
```

## Configure

Generate build configuration using `ccache`

```bash
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
-DCCACHE_PATH=/usr/local/bin/ccache \
-DCMAKE_TOOLCHAIN_FILE=../ydb-cpp-sdk/clang.toolchain \
../ydb-cpp-sdk
```

## Build

```bash
cd build
ninja
```

## Test

```bash
cd build
ctest -j32 --timeout 1200 --output-on-failure
```
