add_library(impl-ydb_internal-common)

target_link_libraries(impl-ydb_internal-common PUBLIC
  yutil
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-common PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/impl/ydb_internal/common/CMakeLists.txt
  parser.cpp
  getenv.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/common/parser.cpp
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/common/getenv.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/common/CMakeLists.txt
=======
  parser.cpp
  getenv.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/common/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


