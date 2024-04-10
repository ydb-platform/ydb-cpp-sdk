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
<<<<<<< HEAD:src/client/proto/CMakeLists.txt
  accessor.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_proto/accessor.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_proto/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Proto ALIAS cpp-client-ydb_proto)
