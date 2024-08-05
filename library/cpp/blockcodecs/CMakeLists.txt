_ydb_sdk_add_library(blockcodecs-core)
target_link_libraries(blockcodecs-core
  PUBLIC
    yutil
)
target_sources(blockcodecs-core PRIVATE
  core/codecs.cpp
  core/stream.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-core)


_ydb_sdk_add_library(blockcodecs-codecs-brotli INTERFACE)
add_global_library_for(blockcodecs-codecs-brotli.global blockcodecs-codecs-brotli)
target_link_libraries(blockcodecs-codecs-brotli.global
  PRIVATE
    yutil
    Brotli::enc
    Brotli::dec
    blockcodecs-core
)
target_sources(blockcodecs-codecs-brotli.global
  PRIVATE
    codecs/brotli/brotli.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-brotli.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-brotli)


_ydb_sdk_add_library(blockcodecs-codecs-bzip INTERFACE)
add_global_library_for(blockcodecs-codecs-bzip.global blockcodecs-codecs-bzip)
target_link_libraries(blockcodecs-codecs-bzip.global 
  PRIVATE
    yutil
    BZip2::BZip2
    blockcodecs-core
)
target_sources(blockcodecs-codecs-bzip.global
  PRIVATE
    codecs/bzip/bzip.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-bzip.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-bzip)


_ydb_sdk_add_library(blockcodecs-codecs-fastlz INTERFACE)
add_global_library_for(blockcodecs-codecs-fastlz.global blockcodecs-codecs-fastlz)
target_link_libraries(blockcodecs-codecs-fastlz.global
  PRIVATE
    yutil
    FastLZ
    blockcodecs-core
)
target_sources(blockcodecs-codecs-fastlz.global
  PRIVATE
    codecs/fastlz/fastlz.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-fastlz.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-fastlz)


_ydb_sdk_add_library(blockcodecs-codecs-lz4 INTERFACE)
add_global_library_for(blockcodecs-codecs-lz4.global blockcodecs-codecs-lz4)
target_link_libraries(blockcodecs-codecs-lz4.global
  PRIVATE
    yutil
    LZ4::LZ4
    blockcodecs-core
)
target_sources(blockcodecs-codecs-lz4.global
  PRIVATE
    codecs/lz4/lz4.cpp
)
target_compile_options(blockcodecs-codecs-lz4.global
  PRIVATE
    -Wno-deprecated
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-lz4.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-lz4)


_ydb_sdk_add_library(blockcodecs-codecs-lzma INTERFACE)
add_global_library_for(blockcodecs-codecs-lzma.global blockcodecs-codecs-lzma)
target_link_libraries(blockcodecs-codecs-lzma.global
  PRIVATE
    yutil
    contrib-libs-lzmasdk
    blockcodecs-core
)
target_sources(blockcodecs-codecs-lzma.global
  PRIVATE
    codecs/lzma/lzma.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-lzma.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-lzma)


_ydb_sdk_add_library(blockcodecs-codecs-snappy INTERFACE)
add_global_library_for(blockcodecs-codecs-snappy.global blockcodecs-codecs-snappy)
target_link_libraries(blockcodecs-codecs-snappy.global
  PRIVATE
    yutil
    Snappy::snappy
    blockcodecs-core
)
target_sources(blockcodecs-codecs-snappy.global
  PRIVATE
    codecs/snappy/snappy.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-snappy.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-snappy)


_ydb_sdk_add_library(blockcodecs-codecs-zlib INTERFACE)
add_global_library_for(blockcodecs-codecs-zlib.global blockcodecs-codecs-zlib)
target_link_libraries(blockcodecs-codecs-zlib.global
  PRIVATE
    yutil
    ZLIB::ZLIB
    blockcodecs-core
)
target_sources(blockcodecs-codecs-zlib.global
  PRIVATE
    codecs/zlib/zlib.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-zlib.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-zlib)


_ydb_sdk_add_library(blockcodecs-codecs-zstd INTERFACE)
add_global_library_for(blockcodecs-codecs-zstd.global blockcodecs-codecs-zstd)
target_link_libraries(blockcodecs-codecs-zstd.global
  PRIVATE
    yutil
    ZSTD::ZSTD
    blockcodecs-core
)
target_sources(blockcodecs-codecs-zstd.global
  PRIVATE
    codecs/zstd/zstd.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-zstd.global)
_ydb_sdk_install_targets(TARGETS blockcodecs-codecs-zstd)


_ydb_sdk_add_library(blockcodecs)
target_link_libraries(blockcodecs 
  PUBLIC
    yutil
    blockcodecs-core
    blockcodecs-codecs-brotli
    blockcodecs-codecs-bzip
    blockcodecs-codecs-fastlz
    blockcodecs-codecs-lz4
    blockcodecs-codecs-lzma
    blockcodecs-codecs-snappy
    blockcodecs-codecs-zlib
    blockcodecs-codecs-zstd
)
target_sources(blockcodecs 
  PRIVATE
    codecs.cpp
    stream.cpp
)
_ydb_sdk_install_targets(TARGETS blockcodecs)
