add_library(internal-proxies-sse41)

target_link_libraries(internal-proxies-sse41 PUBLIC  
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources_custom(internal-proxies-sse41
  .sse41
  SRCS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/digest/argonish/internal/proxies/sse41/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/sse41/proxy_sse41.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/argonish/internal/proxies/sse41/proxy_sse41.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/internal/proxies/sse41/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/sse41/proxy_sse41.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/internal/proxies/sse41/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/sse41/proxy_sse41.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/sse41/proxy_sse41.cpp
>>>>>>> origin/main
  CUSTOM_FLAGS
  -msse4.1
)
