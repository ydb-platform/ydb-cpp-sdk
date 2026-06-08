# YDB C++ SDK: driver for [YDB](https://github.com/ydb-platform/ydb)

## Building YDB C++ SDK from sources

### Prerequisites

- cmake 3.22+
- clang 16+
- git 2.20+
- ninja 1.10+
- ragel
- yasm
- protoc

### Library dependencies

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
- brotli 1.1.0+
- double-conversion
- jwt-cpp

### Runtime requirements

- libidn11-dev (IDN)
- libiconv (Iconv)

### Testing requirements

- gtest
- gmock

### Install dependencies

When building with `YDB_SDK_ODBC=ON` (included in the `release-test-*` presets), the ODBC
driver is a shared library. Static dependencies installed under `~/ydb_deps` must be built
with position-independent code (`-DCMAKE_POSITION_INDEPENDENT_CODE=ON`).

```bash
sudo apt-get -y update
sudo apt-get -y install git gdb ninja-build libidn11-dev ragel yasm libc-ares-dev libre2-dev \
  rapidjson-dev zlib1g-dev libxxhash-dev libzstd-dev libsnappy-dev libgtest-dev libgmock-dev \
  libbz2-dev liblz4-dev libdouble-conversion-dev libssl-dev libstdc++-13-dev gcc-13 g++-13 \
  unixodbc unixodbc-dev

PIC="-DCMAKE_POSITION_INDEPENDENT_CODE=ON"

wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 16

# Install abseil-cpp
wget -O abseil-cpp-20230802.0.tar.gz https://github.com/abseil/abseil-cpp/archive/refs/tags/20230802.0.tar.gz
tar -xvzf abseil-cpp-20230802.0.tar.gz
cd abseil-cpp-20230802.0
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DABSL_PROPAGATE_CXX_STD=ON ${PIC} ..
cmake --build . --config Release
cmake --install . --config Release --prefix ~/ydb_deps/absl
cd ../../

# Install protobuf
wget -O protobuf-3.21.12.tar.gz https://github.com/protocolbuffers/protobuf/archive/refs/tags/v3.21.12.tar.gz
tar -xvzf protobuf-3.21.12.tar.gz
cd protobuf-3.21.12
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_INSTALL=ON ${PIC} ..
cmake --build . --config Release
cmake --install . --config Release --prefix ~/ydb_deps/protobuf
cd ../../

# Install gRPC
wget -O grpc-1.54.3.tar.gz https://github.com/grpc/grpc/archive/refs/tags/v1.54.3.tar.gz
tar -xvzf grpc-1.54.3.tar.gz && cd grpc-1.54.3
mkdir build && cd build
cmake -G Ninja -DCMAKE_PREFIX_PATH="${HOME}/ydb_deps/absl;${HOME}/ydb_deps/protobuf" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 ${PIC} \
  -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DgRPC_BUILD_CSHARP_EXT=OFF \
  -DgRPC_ZLIB_PROVIDER=package -DgRPC_CARES_PROVIDER=package -DgRPC_RE2_PROVIDER=package \
  -DgRPC_SSL_PROVIDER=package -DgRPC_PROTOBUF_PROVIDER=package -DgRPC_ABSL_PROVIDER=package \
  -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
  -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF ..
cmake --build . --config Release
cmake --install . --config Release --prefix ~/ydb_deps/grpc
cd ../../

# Install base64
wget -O base64-0.5.2.tar.gz https://github.com/aklomp/base64/archive/refs/tags/v0.5.2.tar.gz
tar -xvzf base64-0.5.2.tar.gz && cd base64-0.5.2
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ${PIC} ..
cmake --build . --config Release
cmake --install . --config Release --prefix ~/ydb_deps/base64
cd ../../

# Install brotli
wget -O brotli-1.1.0.tar.gz https://github.com/google/brotli/archive/refs/tags/v1.1.0.tar.gz
tar -xvzf brotli-1.1.0.tar.gz && cd brotli-1.1.0
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ${PIC} ..
cmake --build . --config Release
cmake --install . --config Release --prefix ~/ydb_deps/brotli
cd ../../

# Install jwt-cpp
wget -O jwt-cpp-0.7.0.tar.gz https://github.com/Thalhammer/jwt-cpp/archive/refs/tags/v0.7.0.tar.gz
tar -xvzf jwt-cpp-0.7.0.tar.gz && cd jwt-cpp-0.7.0
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ${PIC} ..
cmake --build . --config Release
cmake --install . --config Release --prefix ~/ydb_deps/jwt-cpp
cd ../../
```

### Create the work directory

```bash
mkdir ~/ydbwork && cd ~/ydbwork
mkdir build
```

### Install ccache

Install `ccache` into `/usr/local/bin/`. The recommended version is `4.8.1` or above, the minimum required version is `4.7`.

```bash
(V=4.8.1; curl -L https://github.com/ccache/ccache/releases/download/v${V}/ccache-${V}-linux-x86_64.tar.xz | \
sudo tar -xJ -C /usr/local/bin/ --strip-components=1 --no-same-owner ccache-${V}-linux-x86_64/ccache)
```

### Clone the ydb-cpp-sdk repository

```bash
git clone --recurse-submodules https://github.com/ydb-platform/ydb-cpp-sdk.git
```

### Configure

Generate build configuration using a configure preset (e.g. `release-test-clang`). `ccache` is located automatically, but if you get the warning that it's not been found, specify its location by passing `-DCCACHE_PATH=path/to/bin`

```bash
cd ydb-cpp-sdk
cmake --preset $sdk_configure_preset
```

### Build

```bash
cmake --build --preset $sdk_configure_preset
```

### Build `.deb` packages

The SDK can be packaged as Debian development packages with CPack. The packaging build uses static libraries and produces the following packages:

- `libydb-cpp-dev` — core SDK static library, public headers and CMake package files;
- `libydb-cpp-iam-dev` — IAM credentials plugin;
- `libydb-cpp-otel-metrics-dev` — OpenTelemetry metrics plugin (includes vendored opentelemetry-cpp);
- `libydb-cpp-otel-tracing-dev` — OpenTelemetry tracing plugin (requires `libydb-cpp-otel-metrics-dev` for OTel headers/libs).

The Debian packaging flow is intended for Ubuntu 24.04. Install the regular build dependencies first. OpenTelemetry plugins use the vendored `third_party/opentelemetry-cpp` submodule (v1.26.0, matching the YDB monorepo pin); initialize it with `git submodule update --init third_party/opentelemetry-cpp` before building.

To build `.deb` packages directly with CPack:

```bash
cmake --preset package-deb-clang
cmake --build build-deb --target package -j$(nproc)
```

The generated `.deb` files are placed into `build-deb/`.

To prepare Debian source-package metadata, run:

```bash
./scripts/generate-debian-directory.sh
```

This updates `debian/changelog` from `src/version.h` and the current Git commit. The `debian/` directory contains package metadata and install manifests for `dpkg-buildpackage`/PPA builds.

To validate a binary Debian package build in an Ubuntu 24.04 Docker container:

```bash
./scripts/test_dpkg_buildpackage.sh
```

To smoke-test generated `.deb` packages with the sample consumer project:

```bash
./scripts/test_deb_packages.sh build-deb
```

### Install from GitHub releases

Pre-built `.deb` packages for Ubuntu 24.04 (Noble) are attached to each
GitHub release. Download the assets and install them with `dpkg`:

```bash
# Replace <TAG> with the desired release tag (e.g. v1.2.3)
TAG=<TAG>
BASE="https://github.com/ydb-platform/ydb-cpp-sdk/releases/download/${TAG}"

wget "${BASE}/yandex-googleapis-api-common-protos_1.0.0_amd64.deb"
wget "${BASE}/libydb-cpp-dev_${TAG#v}_amd64.deb"
# Optional plugins:
wget "${BASE}/libydb-cpp-iam-dev_${TAG#v}_amd64.deb"
wget "${BASE}/libydb-cpp-otel-metrics-dev_${TAG#v}_amd64.deb"
wget "${BASE}/libydb-cpp-otel-tracing-dev_${TAG#v}_amd64.deb"

sudo dpkg -i yandex-googleapis-api-common-protos_*.deb
sudo dpkg -i libydb-cpp-dev_*.deb libydb-cpp-iam-dev_*.deb \
             libydb-cpp-otel-metrics-dev_*.deb libydb-cpp-otel-tracing-dev_*.deb
sudo apt-get install -f   # resolve any remaining dependencies
```

After installation, use the SDK in your CMake project:

```cmake
find_package(ydb-cpp-sdk REQUIRED COMPONENTS Driver Table Topic)
target_link_libraries(myapp PRIVATE YDB-CPP-SDK::Driver YDB-CPP-SDK::Table)
```

Pass `-DCMAKE_PREFIX_PATH=/usr/share/yandex` since the packages install
under the Yandex prefix.

### Test

Specify a level of parallelism by passing the `-j<level>` option into the command below (e.g. `-j$(nproc)`)

Running all tests:

```bash
ctest -j$(nproc) --preset all
```

Running unit tests only:

```bash
ctest -j$(nproc) --preset unit
```

Running integration tests only:

```bash
ctest -j$(nproc) --preset integration
```

### Presets

#### Configure presets

| Preset | Build type | Compiler | Tests & examples |
|---|---|---|---|
| `release-clang` | Release | Clang | No |
| `release-gcc` | Release | GCC | No |
| `release-test-clang` | Release | Clang | Yes |
| `release-test-gcc` | Release | GCC | Yes |
| `debug-clang` | Debug | Clang | No |
| `debug-gcc` | Debug | GCC | No |
| `debug-test-clang` | Debug | Clang | Yes |
| `debug-test-gcc` | Debug | GCC | Yes |

#### Test presets

| Preset | Tests included | Requires YDB server |
|---|---|---|
| `all` | Unit + Integration | Yes |
| `unit` | Unit only | No |
| `integration` | Integration only | Yes |

Note that some tests use a legacy test library instead of GoogleTest, see `./<test_target> --help` for details. If you need to run only certain test cases, here is an alternative for `--gtest_filter` option:

```bash
cat <<EOF | ./<test_target> --filter-file /dev/fd/0
-ExcludedTestCase
+IncludedTestCase
+IncludedTestCase::TestName
EOF
```
