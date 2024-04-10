add_library(cpp-client-ydb_import)

target_link_libraries(cpp-client-ydb_import PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  api-grpc
  api-protos
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_proto
  client-ydb_types-operation
)

target_sources(cpp-client-ydb_import PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/import/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/import/CMakeLists.txt
  import.cpp
)

generate_enum_serilization(cpp-client-ydb_import
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/import/import.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_import/import.cpp
)

generate_enum_serilization(cpp-client-ydb_import
  ${YDB_SDK_SOURCE_DIR}/client/ydb_import/import.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_import/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_import/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/import/import.h
========
  import.cpp
)

generate_enum_serilization(cpp-client-ydb_import
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_import/import.h
  INCLUDE_HEADERS
  src/client/ydb_import/import.h
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_import/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Import ALIAS cpp-client-ydb_import)
