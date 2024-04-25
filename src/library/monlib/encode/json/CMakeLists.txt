add_library(monlib-encode-json)

target_link_libraries(monlib-encode-json PUBLIC
  yutil
  cpp-monlib-encode
  monlib-encode-buffered
  cpp-monlib-exception
  library-cpp-json
  cpp-json-writer
)

target_sources(monlib-encode-json PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/monlib/encode/json/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_encoder.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/json/json_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/json/json_encoder.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/json/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_encoder.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/json/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_encoder.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_encoder.cpp
>>>>>>> origin/main
)


