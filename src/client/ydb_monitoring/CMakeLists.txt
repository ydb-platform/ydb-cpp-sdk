add_library(cpp-client-ydb_monitoring)

target_link_libraries(cpp-client-ydb_monitoring PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-client-ydb_proto
  impl-ydb_internal-make_request
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_monitoring PRIVATE
<<<<<<<< HEAD:src/client/monitoring/CMakeLists.txt
<<<<<<< HEAD:src/client/monitoring/CMakeLists.txt
  monitoring.cpp
)

generate_enum_serilization(cpp-client-ydb_monitoring
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/monitoring/monitoring.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_monitoring/monitoring.cpp
)

generate_enum_serilization(cpp-client-ydb_monitoring
  ${YDB_SDK_SOURCE_DIR}/client/ydb_monitoring/monitoring.h
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_monitoring/CMakeLists.txt
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/monitoring/monitoring.h
========
  monitoring.cpp
)

generate_enum_serilization(cpp-client-ydb_monitoring
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_monitoring/monitoring.h
  INCLUDE_HEADERS
  src/client/ydb_monitoring/monitoring.h
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_monitoring/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Monitoring ALIAS cpp-client-ydb_monitoring)
