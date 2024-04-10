add_library(ydb_types-credentials-login)

target_link_libraries(ydb_types-credentials-login PUBLIC
  yutil
  library-login
  library-security
  api-grpc
  client-ydb_types-status
  impl-ydb_internal-grpc_connections
  yql-public-issue
)

target_sources(ydb_types-credentials-login PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/types/credentials/login/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/types/credentials/login/CMakeLists.txt
  login.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/credentials/login/login.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/credentials/login/CMakeLists.txt
========
  login.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/credentials/login/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/credentials/login/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


