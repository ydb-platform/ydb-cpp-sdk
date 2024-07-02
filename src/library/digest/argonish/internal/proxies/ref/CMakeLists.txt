add_library(internal-proxies-ref)

target_link_libraries(internal-proxies-ref PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources(internal-proxies-ref PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ref/proxy_ref.cpp
)

_ydb_sdk_install_targets(TARGETS internal-proxies-ref)
