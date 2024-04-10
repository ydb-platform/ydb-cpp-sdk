add_library(impl-ydb_internal-plain_status)

target_link_libraries(impl-ydb_internal-plain_status PUBLIC
  yutil
  protobuf::libprotobuf
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-plain_status PRIVATE
<<<<<<< HEAD:src/client/impl/ydb_internal/plain_status/CMakeLists.txt
  status.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/plain_status/status.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/plain_status/CMakeLists.txt
)
