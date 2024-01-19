# Building YDB C++ SDK from sources

## Prerequisites

 - cmake 3.22+
 - clang-14
 - lld-14
 - git 2.20+
 - ninja 1.10+

## Install dependencies

```bash
sudo apt-get -y install git cmake ninja-build clang-14 lld-14 libidn11-dev llvm-14
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
ninja client/all examples/all
```
