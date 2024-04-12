add_subdirectory(impl)
add_subdirectory(query_stats)

add_library(cpp-client-ydb_table)

target_link_libraries(cpp-client-ydb_table PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  api-protos
  impl-ydb_internal-make_request
  impl-ydb_internal-kqp_session_common
  impl-ydb_internal-retry
  cpp-client-ydb_driver
  cpp-client-ydb_params
  cpp-client-ydb_proto
  cpp-client-ydb_result
  cpp-client-ydb_scheme
  client-ydb_table-impl
  client-ydb_table-query_stats
  client-ydb_types-operation
  cpp-client-ydb_value
  cpp-string_utils-misc
)

target_sources(cpp-client-ydb_table PRIVATE
<<<<<<<< HEAD:src/client/table/CMakeLists.txt
<<<<<<< HEAD:src/client/table/CMakeLists.txt
  table.cpp
  proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_table
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/table/table_enum.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/table.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_table
  ${YDB_SDK_SOURCE_DIR}/client/ydb_table/table_enum.h
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_table/CMakeLists.txt
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/table/table_enum.h
========
  table.cpp
  proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_table
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_table/table_enum.h
  INCLUDE_HEADERS
  src/client/ydb_table/table_enum.h
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Table ALIAS cpp-client-ydb_table)
