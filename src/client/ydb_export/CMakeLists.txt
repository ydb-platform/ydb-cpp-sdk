add_library(cpp-client-ydb_export)

target_link_libraries(cpp-client-ydb_export PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  api-grpc
  api-protos
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_proto
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_export PRIVATE
<<<<<<<< HEAD:src/client/export/CMakeLists.txt
<<<<<<< HEAD:src/client/export/CMakeLists.txt
  export.cpp
)

generate_enum_serilization(cpp-client-ydb_export
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/export/export.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_export/export.cpp
)

generate_enum_serilization(cpp-client-ydb_export
  ${YDB_SDK_SOURCE_DIR}/client/ydb_export/export.h
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_export/CMakeLists.txt
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/export/export.h
========
  export.cpp
)

generate_enum_serilization(cpp-client-ydb_export
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_export/export.h
  INCLUDE_HEADERS
  src/client/ydb_export/export.h
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_export/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Export ALIAS cpp-client-ydb_export)
