add_library(cpp-client-ydb_proto)

target_link_libraries(cpp-client-ydb_proto PUBLIC
  yutil
  api-grpc
  api-grpc-draft
  api-protos
  lib-operation_id-protos
  cpp-client-ydb_params
  cpp-client-ydb_value
  public-issue-protos
)

target_sources(cpp-client-ydb_proto PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/proto/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/proto/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/proto/CMakeLists.txt
  accessor.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_proto/accessor.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_proto/CMakeLists.txt
========
  accessor.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_proto/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_proto/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  accessor.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_proto/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  accessor.cpp
>>>>>>> origin/main
)

add_library(YDB-CPP-SDK::Proto ALIAS cpp-client-ydb_proto)
