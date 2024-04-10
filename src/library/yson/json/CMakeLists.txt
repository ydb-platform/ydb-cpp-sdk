add_library(cpp-yson-json)

target_link_libraries(cpp-yson-json PUBLIC
  yutil
  library-cpp-json
)

target_sources(cpp-yson-json PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/yson/json/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/yson2json_adapter.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yson/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yson/json/yson2json_adapter.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yson/json/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/yson2json_adapter.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yson/json/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


