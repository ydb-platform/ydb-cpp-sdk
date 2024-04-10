add_library(internal-proxies-ref)

target_link_libraries(internal-proxies-ref PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources(internal-proxies-ref PRIVATE
<<<<<<< HEAD:src/library/digest/argonish/internal/proxies/ref/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ref/proxy_ref.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/argonish/internal/proxies/ref/proxy_ref.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/internal/proxies/ref/CMakeLists.txt
)
