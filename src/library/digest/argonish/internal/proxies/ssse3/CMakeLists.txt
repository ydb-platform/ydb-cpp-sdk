add_library(internal-proxies-ssse3)

target_link_libraries(internal-proxies-ssse3 PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources_custom(internal-proxies-ssse3
  .ssse3
  SRCS
<<<<<<< HEAD
<<<<<<< HEAD:src/library/digest/argonish/internal/proxies/ssse3/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ssse3/proxy_ssse3.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/argonish/internal/proxies/ssse3/proxy_ssse3.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/internal/proxies/ssse3/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ssse3/proxy_ssse3.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  CUSTOM_FLAGS
  -mssse3
)
