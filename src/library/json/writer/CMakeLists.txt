add_library(cpp-json-writer)

target_link_libraries(cpp-json-writer PUBLIC
  
  yutil
  cpp-json-common
  tools-enum_parser-enum_serialization_runtime
  cpp-string_utils-misc
)

target_sources(cpp-json-writer PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/json/writer/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/json/writer/json_value.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/writer/json.cpp
)

generate_enum_serilization(cpp-json-writer
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json/writer/json_value.h
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/writer/json_value.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/writer/json.cpp
)

generate_enum_serilization(cpp-json-writer
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/writer/json_value.h
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/writer/CMakeLists.txt
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/json/writer/json_value.h
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/json/writer/json_value.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/writer/json.cpp
)

generate_enum_serilization(cpp-json-writer
  ${YDB_SDK_SOURCE_DIR}/src/library/json/writer/json_value.h
  INCLUDE_HEADERS
  src/library/json/writer/json_value.h
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


