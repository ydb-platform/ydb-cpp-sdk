# YDB C++ SDK: driver for [YDB](https://github.com/ydb-platform/ydb)

[![Codecov](https://codecov.io/gh/ydb-platform/ydb-cpp-sdk/branch/main/graph/badge.svg)](https://app.codecov.io/gh/ydb-platform/ydb-cpp-sdk)

## Building YDB C++ SDK from sources

### Prerequisites

- cmake 3.22+
- a current C++20 compiler (GCC or Clang)
- git 2.20+
- ninja 1.10+
- ragel
- yasm
- OpenSSL
- Iconv
- IDN

### Install dependencies

CPM fetches the pinned library dependencies during CMake configure. The only
packages to install manually are the tools and platform prerequisites above.
The authoritative pins are in `cmake/dependencies.cmake`.

Ubuntu 24.04:

```bash
sudo apt-get -y update
sudo apt-get -y install build-essential ca-certificates ccache clang cmake git \
  libidn11-dev libssl-dev lld ninja-build pkg-config python3 ragel yasm
```

Fedora 43:

```bash
sudo dnf install -y ccache cmake gcc gcc-c++ git libidn-devel \
  ninja-build openssl-devel openssl-devel-engine pkgconf-pkg-config python3 ragel yasm
```

macOS 14:

```bash
brew install ccache cmake git libidn llvm ninja openssl@3 python ragel yasm
export PATH="$(brew --prefix llvm)/bin:$PATH"
```

### Clone the ydb-cpp-sdk repository

```bash
git clone https://github.com/ydb-platform/ydb-cpp-sdk.git
```

### Configure

Generate build configuration using a configure preset (e.g. `release-test-clang`). `ccache` is located automatically, but if you get the warning that it's not been found, specify its location by passing `-DCCACHE_PATH=path/to/bin`

```bash
cd ydb-cpp-sdk
cmake --preset $sdk_configure_preset
```

`YDB_SDK_DEPENDENCY_MODE` defaults to `CPM`. Set `CPM_SOURCE_CACHE` to a
reusable directory before configuring; a fully populated cache can be reused
offline with `FETCHCONTENT_FULLY_DISCONNECTED=ON`. `SYSTEM` mode is intended
only for Ubuntu DEB builds.

With the Dev Container CLI, `devcontainer up --workspace-folder .` creates the
development container and configures the test build through the same CPM path.

### Build

```bash
cmake --build build --parallel
```

### Build `.deb` packages

The SDK can be packaged as Debian development packages with CPack. The complete packaging flow uses static libraries and produces the following packages:

- `yandex-googleapis-api-common-protos` — generated API Common Protos headers and static library, required by `libydb-cpp-dev`;
- `libydb-cpp-dev` — core SDK static library, public headers and CMake package files;
- `libydb-cpp-iam-dev` — IAM credentials plugin;
- `libydb-cpp-otel-metrics-dev` — OpenTelemetry metrics plugin;
- `libydb-cpp-otel-tracing-dev` — OpenTelemetry tracing plugin (requires `libydb-cpp-otel-metrics-dev` for OTel headers/libs).

The CPack-only packaging flow is intended for Ubuntu 24.04. It builds and
installs the Google common-protos package before packaging the four SDK
components, so all five packages use the distro protobuf ABI:

```bash
./scripts/build_cpack_deb_packages.sh build-deb/packages
```

The generated `.deb` files are placed into `build-deb/packages/` and install
under `/usr/share/yandex`. The IAM and OTel packages require the matching core
version; tracing additionally requires the matching metrics package.

To smoke-test generated `.deb` packages with the sample consumer project:

```bash
./scripts/test_deb_packages.sh build-deb/packages
```

### Install from GitHub releases

Pre-built `.deb` packages for Ubuntu 24.04 (Noble) are attached to each
GitHub release. Download the assets and install them with APT:

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

sudo apt-get update
sudo apt-get install -y \
    ./yandex-googleapis-api-common-protos_*.deb \
    ./libydb-cpp-dev_*.deb ./libydb-cpp-iam-dev_*.deb \
    ./libydb-cpp-otel-metrics-dev_*.deb ./libydb-cpp-otel-tracing-dev_*.deb
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
