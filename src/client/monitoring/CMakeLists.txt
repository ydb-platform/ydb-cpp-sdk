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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/monitoring/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/monitoring/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/monitoring/CMakeLists.txt
=======
>>>>>>> origin/main
  monitoring.cpp
)

generate_enum_serilization(cpp-client-ydb_monitoring
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/monitoring/monitoring.h
<<<<<<< HEAD
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_monitoring/monitoring.cpp
)

generate_enum_serilization(cpp-client-ydb_monitoring
  ${YDB_SDK_SOURCE_DIR}/client/ydb_monitoring/monitoring.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_monitoring/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_monitoring/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/monitoring/monitoring.h
========
  monitoring.cpp
)

generate_enum_serilization(cpp-client-ydb_monitoring
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_monitoring/monitoring.h
  INCLUDE_HEADERS
  src/client/ydb_monitoring/monitoring.h
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_monitoring/CMakeLists.txt
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_monitoring/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/monitoring/monitoring.h
>>>>>>> origin/main
)

add_library(YDB-CPP-SDK::Monitoring ALIAS cpp-client-ydb_monitoring)
