add_library(public-lib-json_value)

target_link_libraries(public-lib-json_value PUBLIC
  yutil
  cpp-json-writer
  cpp-string_utils-base64
  cpp-client-ydb_result
  cpp-client-ydb_value
  library-uuid
)

target_sources(public-lib-json_value PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/json_value/CMakeLists.txt
  ydb_json_value.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/public/lib/json_value/ydb_json_value.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/lib/json_value/CMakeLists.txt
=======
  ydb_json_value.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/public/lib/json_value/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_library(YDB-CPP-SDK::JsonValue ALIAS public-lib-json_value)
