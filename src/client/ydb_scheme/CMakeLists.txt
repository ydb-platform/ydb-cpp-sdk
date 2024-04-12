add_library(cpp-client-ydb_scheme)

target_link_libraries(cpp-client-ydb_scheme PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  impl-ydb_internal-make_request
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_scheme PRIVATE
<<<<<<<< HEAD:src/client/scheme/CMakeLists.txt
<<<<<<< HEAD:src/client/scheme/CMakeLists.txt
  scheme.cpp
)

generate_enum_serilization(cpp-client-ydb_scheme
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/scheme/scheme.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_scheme/scheme.cpp
)

generate_enum_serilization(cpp-client-ydb_scheme
  ${YDB_SDK_SOURCE_DIR}/client/ydb_scheme/scheme.h
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_scheme/CMakeLists.txt
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/scheme/scheme.h
========
  scheme.cpp
)

generate_enum_serilization(cpp-client-ydb_scheme
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_scheme/scheme.h
  INCLUDE_HEADERS
  src/client/ydb_scheme/scheme.h
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_scheme/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Scheme ALIAS cpp-client-ydb_scheme)
