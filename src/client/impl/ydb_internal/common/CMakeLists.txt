add_library(impl-ydb_internal-common)

target_link_libraries(impl-ydb_internal-common PUBLIC
  yutil
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-common PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/client/impl/ydb_internal/common/CMakeLists.txt
  parser.cpp
  getenv.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/common/parser.cpp
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/common/getenv.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/common/CMakeLists.txt
=======
  parser.cpp
  getenv.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


