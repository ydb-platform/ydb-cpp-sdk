add_library(monlib-encode-buffered)

target_link_libraries(monlib-encode-buffered PUBLIC
  yutil
  cpp-monlib-encode
  cpp-monlib-metrics
)

target_sources(monlib-encode-buffered PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/monlib/encode/buffered/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/buffered/string_pool.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/buffered/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


