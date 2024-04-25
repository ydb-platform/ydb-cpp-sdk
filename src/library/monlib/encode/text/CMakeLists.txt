add_library(monlib-encode-text)

target_link_libraries(monlib-encode-text PUBLIC
  yutil
  cpp-monlib-encode
)

target_sources(monlib-encode-text PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/monlib/encode/text/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/text/text_encoder.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/text/text_encoder.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/text/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/text/text_encoder.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/text/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/text/text_encoder.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/text/text_encoder.cpp
>>>>>>> origin/main
)
