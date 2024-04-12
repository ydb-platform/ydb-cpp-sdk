add_library(cpp-client-ydb_value)

target_link_libraries(cpp-client-ydb_value PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-containers-stack_vector
  api-protos
  impl-ydb_internal-value_helpers
  client-ydb_types-fatal_error_handlers
  yql-public-decimal
  library-uuid
  proto_output
)

target_sources(cpp-client-ydb_value PRIVATE
<<<<<<<< HEAD:src/client/value/CMakeLists.txt
<<<<<<< HEAD:src/client/value/CMakeLists.txt
  value.cpp
)

generate_enum_serilization(cpp-client-ydb_value
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/value/value.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value/value.cpp
)

generate_enum_serilization(cpp-client-ydb_value
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value/value.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_value/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_value/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/value/value.h
========
  value.cpp
)

generate_enum_serilization(cpp-client-ydb_value
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_value/value.h
  INCLUDE_HEADERS
  src/client/ydb_value/value.h
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_value/CMakeLists.txt
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_value/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::Value ALIAS cpp-client-ydb_value)
