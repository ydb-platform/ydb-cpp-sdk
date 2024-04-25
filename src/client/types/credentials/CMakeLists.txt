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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/types/credentials/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/types/credentials/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/types/credentials/CMakeLists.txt
  credentials.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/credentials/credentials.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/credentials/CMakeLists.txt
========
  credentials.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/credentials/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/credentials/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  credentials.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/credentials/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  credentials.cpp
>>>>>>> origin/main
)


