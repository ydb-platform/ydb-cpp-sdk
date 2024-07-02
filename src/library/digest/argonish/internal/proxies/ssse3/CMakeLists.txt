add_library(internal-proxies-ssse3)

target_link_libraries(internal-proxies-ssse3 PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources_custom(internal-proxies-ssse3
  .ssse3
  SRCS
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ssse3/proxy_ssse3.cpp
  CUSTOM_FLAGS
  -mssse3
)

_ydb_sdk_install_targets(TARGETS internal-proxies-ssse3)
