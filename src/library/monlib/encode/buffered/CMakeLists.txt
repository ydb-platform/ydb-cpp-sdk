add_library(monlib-encode-buffered)

target_link_libraries(monlib-encode-buffered PUBLIC
  yutil
  cpp-monlib-encode
  cpp-monlib-metrics
)

target_sources(monlib-encode-buffered PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/monlib/encode/buffered/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/buffered/string_pool.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/buffered/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/buffered/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
>>>>>>> origin/main
)


