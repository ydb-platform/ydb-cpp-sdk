add_library(internal-proxies-sse2)

target_link_libraries(internal-proxies-sse2 PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources_custom(internal-proxies-sse2
  .sse2
  SRCS
<<<<<<< HEAD:src/library/digest/argonish/internal/proxies/sse2/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/sse2/proxy_sse2.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/argonish/internal/proxies/sse2/proxy_sse2.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/internal/proxies/sse2/CMakeLists.txt
  CUSTOM_FLAGS
  -msse2
)
