add_library(cpp-client-ydb_scheme)

target_link_libraries(cpp-client-ydb_scheme PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  impl-ydb_internal-make_request
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_scheme PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/scheme/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/scheme/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/scheme/CMakeLists.txt
=======
>>>>>>> origin/main
  scheme.cpp
)

generate_enum_serilization(cpp-client-ydb_scheme
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/scheme/scheme.h
<<<<<<< HEAD
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_scheme/scheme.cpp
)

generate_enum_serilization(cpp-client-ydb_scheme
  ${YDB_SDK_SOURCE_DIR}/client/ydb_scheme/scheme.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_scheme/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_scheme/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/scheme/scheme.h
========
  scheme.cpp
)

generate_enum_serilization(cpp-client-ydb_scheme
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_scheme/scheme.h
  INCLUDE_HEADERS
  src/client/ydb_scheme/scheme.h
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_scheme/CMakeLists.txt
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_scheme/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/scheme/scheme.h
>>>>>>> origin/main
)

add_library(YDB-CPP-SDK::Scheme ALIAS cpp-client-ydb_scheme)
