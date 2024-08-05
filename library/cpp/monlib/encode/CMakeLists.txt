_ydb_sdk_add_library(monlib-encode-buffered)
target_link_libraries(monlib-encode-buffered
  PUBLIC
    yutil
    monlib-encode
    monlib-metrics
)
target_sources(monlib-encode-buffered
  PRIVATE
    buffered/buffered_encoder_base.cpp
    buffered/string_pool.cpp
)
_ydb_sdk_install_targets(TARGETS monlib-encode-buffered)


_ydb_sdk_add_library(monlib-encode-json)
target_link_libraries(monlib-encode-json
  PUBLIC
    yutil
    monlib-encode
    monlib-metrics
  PRIVATE
    monlib-encode-buffered
    monlib-exception
    json
    json-writer
)
target_sources(monlib-encode-json
  PRIVATE
    json/json_decoder.cpp
    json/json_encoder.cpp
)
_ydb_sdk_install_targets(TARGETS monlib-encode-json)


_ydb_sdk_add_library(monlib-encode-prometheus)
target_link_libraries(monlib-encode-prometheus
  PUBLIC
    yutil
    monlib-metrics
  PRIVATE
    monlib-encode
    monlib-encode-buffered
)
target_sources(monlib-encode-prometheus
  PRIVATE
    prometheus/prometheus_decoder.cpp
    prometheus/prometheus_encoder.cpp
)
_ydb_sdk_install_targets(TARGETS monlib-encode-prometheus)


_ydb_sdk_add_library(monlib-encode-spack)
target_link_libraries(monlib-encode-spack
  PUBLIC
    yutil
    monlib-encode
    monlib-metrics
  PRIVATE
    monlib-encode-buffered
    monlib-exception
    LZ4::LZ4
    xxHash::xxHash
    ZLIB::ZLIB
    ZSTD::ZSTD
)
target_sources(monlib-encode-spack
  PRIVATE
    spack/spack_v1_decoder.cpp
    spack/spack_v1_encoder.cpp
    spack/varint.cpp
    spack/compression.cpp
)
_ydb_sdk_install_targets(TARGETS monlib-encode-spack)


_ydb_sdk_add_library(monlib-encode-text)
target_link_libraries(monlib-encode-text
  PUBLIC
    yutil
    monlib-encode
)
target_sources(monlib-encode-text PRIVATE
  text/text_encoder.cpp
)
_ydb_sdk_install_targets(TARGETS monlib-encode-text)


_ydb_sdk_add_library(monlib-encode)
target_link_libraries(monlib-encode
  PUBLIC
    yutil
    monlib-metrics
  PRIVATE
    enum_serialization_runtime
    string_utils-misc
)
target_sources(monlib-encode PRIVATE
  encoder.cpp
  encoder_state.cpp
  format.cpp
)
generate_enum_serilization(monlib-encode
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/encoder_state_enum.h
  GEN_HEADER
  ${YDB_SDK_BINARY_DIR}/library/cpp/monlib/encode/encoder_state_enum.h_serialized.h
  INCLUDE_HEADERS
  library/cpp/monlib/encode/encoder_state_enum.h
)
_ydb_sdk_install_targets(TARGETS monlib-encode)
