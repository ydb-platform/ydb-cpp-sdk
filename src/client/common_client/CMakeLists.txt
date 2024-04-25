add_subdirectory(impl)

add_library(cpp-client-ydb_common_client)

target_link_libraries(cpp-client-ydb_common_client PUBLIC
  yutil
  impl-ydb_internal-common
  client-ydb_types-credentials
)

target_sources(cpp-client-ydb_common_client PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/common_client/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/common_client/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/common_client/CMakeLists.txt
  settings.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_common_client/settings.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_common_client/CMakeLists.txt
========
  settings.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_common_client/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_common_client/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  settings.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_common_client/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  settings.cpp
>>>>>>> origin/main
)

add_library(YDB-CPP-SDK::CommonClient ALIAS cpp-client-ydb_common_client)
