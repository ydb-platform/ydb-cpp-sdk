add_library(internal-proxies-sse41)

target_link_libraries(internal-proxies-sse41 PUBLIC  
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources_custom(internal-proxies-sse41
  .sse41
  SRCS
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/argonish/internal/proxies/sse41/proxy_sse41.cpp
  CUSTOM_FLAGS
  -msse4.1
)
