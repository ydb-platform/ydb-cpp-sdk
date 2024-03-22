add_library(monlib-encode-spack)

target_link_libraries(monlib-encode-spack PUBLIC
  yutil
  monlib-encode-buffered
  cpp-monlib-exception
  LZ4::LZ4
  xxHash::xxHash
  ZLIB::ZLIB
  ZSTD::ZSTD
)

target_sources(monlib-encode-spack PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/spack/spack_v1_decoder.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/spack/spack_v1_encoder.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/spack/varint.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/spack/compression.cpp
)


