add_subdirectory(internal)

add_library(digest-argonish)

target_link_libraries(digest-argonish PUBLIC
  yutil
  threading-poor_man_openmp
  internal-proxies-avx2
  internal-proxies-ref
  internal-proxies-sse2
  internal-proxies-sse41
  internal-proxies-ssse3
)

target_sources(digest-argonish PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/factory/factory.cpp
)

_ydb_sdk_install_targets(TARGETS digest-argonish)
