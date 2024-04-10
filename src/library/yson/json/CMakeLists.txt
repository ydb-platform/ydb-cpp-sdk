add_library(cpp-yson-json)

target_link_libraries(cpp-yson-json PUBLIC
  yutil
  library-cpp-json
)

target_sources(cpp-yson-json PRIVATE
<<<<<<< HEAD:src/library/yson/json/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/yson2json_adapter.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yson/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yson/json/yson2json_adapter.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yson/json/CMakeLists.txt
)


