add_library(cpp-string_utils-stream)

target_link_libraries(cpp-string_utils-stream PUBLIC
  yutil
)

target_sources(cpp-string_utils-stream PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/string_utils/stream/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/stream/stream.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/string_utils/stream/stream.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/string_utils/stream/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/stream/stream.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)

