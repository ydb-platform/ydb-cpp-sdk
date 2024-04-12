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
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
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
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ydb_json_value.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::JsonValue ALIAS public-lib-json_value)
