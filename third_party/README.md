# Third-party dependencies

Vendored libraries used by the YDB C++ SDK. Initialize submodules before building:

```bash
git submodule update --init third_party/aklomp-base64 third_party/jwt-cpp \
  third_party/opentelemetry-cpp third_party/api-common-protos third_party/FastLZ
```

| Dependency | Location | Version / pin | Debian package |
|------------|----------|---------------|----------------|
| aklomp-base64 | `third_party/aklomp-base64` | v0.5.2 (submodule) | `libydb-cpp-dev` |
| jwt-cpp | `third_party/jwt-cpp` | submodule tag | `libydb-cpp-dev` |
| picojson | bundled in `jwt-cpp/include/picojson` | (via jwt-cpp) | `libydb-cpp-dev` |
| opentelemetry-cpp | `third_party/opentelemetry-cpp` | v1.26.0 (submodule) | `libydb-cpp-otel-metrics-dev` |
| api-common-protos | `third_party/api-common-protos` | submodule | built into core / system package |
| FastLZ | `third_party/FastLZ` | submodule | sources only (linked into core) |
| nayuki_md5 | `third_party/nayuki_md5` | in-tree | sources only (linked into core) |

OpenTelemetry is only required when `YDB_SDK_ENABLE_OTEL_METRICS` or `YDB_SDK_ENABLE_OTEL_TRACE` is enabled.
The tracing plugin `.deb` depends on `libydb-cpp-otel-metrics-dev` for vendored OpenTelemetry artifacts.
