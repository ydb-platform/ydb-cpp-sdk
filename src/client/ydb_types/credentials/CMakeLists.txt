add_subdirectory(login)

add_library(client-ydb_types-credentials)

target_link_libraries(client-ydb_types-credentials PUBLIC
  yutil
  library-login
  api-grpc
  client-ydb_types-status
  yql-public-issue
)

target_sources(client-ydb_types-credentials PRIVATE
<<<<<<<< HEAD:src/client/types/credentials/CMakeLists.txt
<<<<<<< HEAD:src/client/types/credentials/CMakeLists.txt
  credentials.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/credentials/credentials.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/credentials/CMakeLists.txt
========
  credentials.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/credentials/CMakeLists.txt
)


