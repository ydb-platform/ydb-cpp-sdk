add_library(internal-proxies-ref)

target_link_libraries(internal-proxies-ref PUBLIC
  internal-proxies-macro
  argonish-internal-argon2
  argonish-internal-blake2b
)

target_sources(internal-proxies-ref PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/digest/argonish/internal/proxies/ref/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ref/proxy_ref.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/digest/argonish/internal/proxies/ref/proxy_ref.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/internal/proxies/ref/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ref/proxy_ref.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/digest/argonish/internal/proxies/ref/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ref/proxy_ref.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/argonish/internal/proxies/ref/proxy_ref.cpp
>>>>>>> origin/main
)
