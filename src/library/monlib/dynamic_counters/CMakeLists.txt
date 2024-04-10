add_subdirectory(percentile)

add_library(cpp-monlib-dynamic_counters)

target_link_libraries(cpp-monlib-dynamic_counters PUBLIC
  yutil
  cpp-containers-stack_vector
  monlib-encode-json
  monlib-encode-spack
  monlib-encode-prometheus
  monlib-service-pages
  cpp-string_utils-quote
  cpp-threading-light_rw_lock
)

target_sources(cpp-monlib-dynamic_counters PRIVATE
<<<<<<< HEAD:src/library/monlib/dynamic_counters/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/counters.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/encode.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/golovan_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/dynamic_counters/page.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/dynamic_counters/counters.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/dynamic_counters/encode.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/dynamic_counters/golovan_page.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/dynamic_counters/page.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/dynamic_counters/CMakeLists.txt
)


