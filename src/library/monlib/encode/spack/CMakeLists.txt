add_library(monlib-encode-spack)

target_link_libraries(monlib-encode-spack PUBLIC
  yutil
  monlib-encode-buffered
  monlib-exception
  LZ4::LZ4
  xxHash::xxHash
  ZLIB::ZLIB
  ZSTD::ZSTD
)

target_sources(monlib-encode-spack PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/spack/spack_v1_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/spack/spack_v1_encoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/spack/varint.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/spack/compression.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-encode-spack)
