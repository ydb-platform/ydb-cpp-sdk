add_library(internal-proxies-avx2)

target_link_libraries(internal-proxies-avx2 PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources_custom(internal-proxies-avx2
  .avx2
  SRCS
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/avx2/proxy_avx2.cpp
  CUSTOM_FLAGS
  -mavx2
  -mfma
  -mbmi
  -mbmi2
)
