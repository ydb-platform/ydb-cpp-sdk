add_library(internal-proxies-sse2)

target_link_libraries(internal-proxies-sse2 PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources_custom(internal-proxies-sse2
  .sse2
  SRCS
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/sse2/proxy_sse2.cpp
  CUSTOM_FLAGS
  -msse2
)

_ydb_sdk_install_targets(TARGETS internal-proxies-sse2)
