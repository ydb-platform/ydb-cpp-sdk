add_subdirectory(credentials)
add_subdirectory(exceptions)
add_subdirectory(fatal_error_handlers)
add_subdirectory(operation)
add_subdirectory(status)

add_library(cpp-client-ydb_types)

target_link_libraries(cpp-client-ydb_types PUBLIC
  yutil
  protobuf::libprotobuf
  library-grpc-client
  yql-public-issue
  tools-enum_parser-enum_serialization_runtime
)

generate_enum_serilization(cpp-client-ydb_types
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/types/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/types/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/types/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/types/s3_settings.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/s3_settings.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/types/s3_settings.h
>>>>>>> origin/main
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/types/s3_settings.h
)

generate_enum_serilization(cpp-client-ydb_types
<<<<<<< HEAD
<<<<<<< HEAD:src/client/types/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/types/status_codes.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/status_codes.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/types/status_codes.h
========
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_types/s3_settings.h
  INCLUDE_HEADERS
  src/client/ydb_types/s3_settings.h
)

generate_enum_serilization(cpp-client-ydb_types
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_types/status_codes.h
  INCLUDE_HEADERS
  src/client/ydb_types/status_codes.h
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/CMakeLists.txt
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/types/status_codes.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/types/status_codes.h
>>>>>>> origin/main
)

add_library(YDB-CPP-SDK::Types ALIAS cpp-client-ydb_types)
