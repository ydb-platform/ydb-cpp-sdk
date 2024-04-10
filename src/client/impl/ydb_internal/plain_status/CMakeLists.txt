add_library(impl-ydb_internal-plain_status)

target_link_libraries(impl-ydb_internal-plain_status PUBLIC
  yutil
  protobuf::libprotobuf
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-plain_status PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/impl/ydb_internal/plain_status/CMakeLists.txt
  status.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/plain_status/status.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/plain_status/CMakeLists.txt
=======
  status.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/plain_status/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
