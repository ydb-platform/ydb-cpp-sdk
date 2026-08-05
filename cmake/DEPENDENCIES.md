# Dependency maintenance

`dependencies.cmake` is the sole authority for fetched dependency versions and
source commits. Do not repeat pins in presets, containers, workflows, or prose.

To update a dependency:

1. Change its pin in `dependencies.cmake` and review the upstream release notes.
2. Configure from an empty build directory and inspect the targets provided by
   the new release. Keep target-name differences inside the manifest's alias
   adapters so SDK CMakeLists and installed `YDB-CPP-SDK::*` interfaces remain
   unchanged.
3. Invalidate only that package's directory in `CPM_SOURCE_CACHE`, or use a new
   empty cache. Never diagnose a pin update using a stale source directory.
4. Compile the complete SDK, examples, test binaries, IAM support, and both OTel
   plugins on Ubuntu 24.04, Fedora 43, and native macOS 14 arm64 using the
   commands below.
5. On Ubuntu, generate all five CPack DEBs and run the clean-container consumer
   smoke test. Full Fedora and macOS test-suite execution is not required.

Commit-pin dependencies such as Google API common protos and FastLZ must remain
full immutable hashes. Keep `CPM.cmake` pinned by version and SHA-256 as well.

## Platform verification

The first configure requires network access. For an offline configure, reuse a
cache populated by a successful configure (including its `cpm/CPM_0.42.0.cmake`
bootstrap) and add `-DFETCHCONTENT_FULLY_DISCONNECTED=ON`.

Ubuntu 24.04:

```bash
docker build -t ydb-cpp-sdk-dev -f .devcontainer/Dockerfile .
docker run --rm -v "$PWD:/src:ro" -v ydb-cpm-ubuntu:/cpm \
  -e CPM_SOURCE_CACHE=/cpm ydb-cpp-sdk-dev bash -lc '
cmake -S /src -B /tmp/build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DYDB_SDK_TESTS=ON -DYDB_SDK_EXAMPLES=ON \
  -DYDB_SDK_ENABLE_OTEL_METRICS=ON -DYDB_SDK_ENABLE_OTEL_TRACE=ON
cmake --build /tmp/build --parallel'
```

Fedora 43:

```bash
docker run --rm -v "$PWD:/src:ro" -v ydb-cpm-fedora:/cpm fedora:43 bash -lc '
dnf install -y ccache cmake gcc gcc-c++ git libidn-devel \
  ninja-build openssl-devel openssl-devel-engine pkgconf-pkg-config python3 ragel yasm
CPM_SOURCE_CACHE=/cpm cmake -S /src -B /tmp/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
  -DYDB_SDK_TESTS=ON -DYDB_SDK_EXAMPLES=ON \
  -DYDB_SDK_ENABLE_OTEL_METRICS=ON -DYDB_SDK_ENABLE_OTEL_TRACE=ON
cmake --build /tmp/build --parallel'
```

Native macOS 14 arm64:

```bash
export PATH="$(brew --prefix llvm)/bin:$PATH"
export CPM_SOURCE_CACHE="$PWD/.cache/cpm"
cmake -S . -B build-macos -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DOPENSSL_ROOT_DIR="$(brew --prefix openssl@3)" \
  -DYDB_SDK_TESTS=ON -DYDB_SDK_EXAMPLES=ON \
  -DYDB_SDK_ENABLE_OTEL_METRICS=ON -DYDB_SDK_ENABLE_OTEL_TRACE=ON
cmake --build build-macos --parallel
```
