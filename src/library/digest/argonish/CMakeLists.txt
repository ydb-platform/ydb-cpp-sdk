add_subdirectory(internal)

add_library(cpp-digest-argonish)

target_link_libraries(cpp-digest-argonish PUBLIC
  yutil
  cpp-threading-poor_man_openmp
  internal-proxies-avx2
  internal-proxies-ref
  internal-proxies-sse2
  internal-proxies-sse41
  internal-proxies-ssse3
)

target_sources(cpp-digest-argonish PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/factory/factory.cpp
)
