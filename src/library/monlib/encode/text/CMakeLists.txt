add_library(monlib-encode-text)

target_link_libraries(monlib-encode-text PUBLIC
  yutil
  cpp-monlib-encode
)

target_sources(monlib-encode-text PRIVATE
<<<<<<< HEAD:src/library/monlib/encode/text/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/text/text_encoder.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/text/text_encoder.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/text/CMakeLists.txt
)
