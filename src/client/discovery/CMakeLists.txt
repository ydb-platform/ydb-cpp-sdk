add_library(cpp-client-ydb_discovery)

target_compile_options(cpp-client-ydb_discovery PRIVATE
  -Wno-deprecated
)

target_link_libraries(cpp-client-ydb_discovery PUBLIC
  yutil
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_discovery PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/discovery/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/discovery/CMakeLists.txt
  discovery.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_discovery/discovery.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_discovery/CMakeLists.txt
========
  discovery.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_discovery/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_discovery/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_library(YDB-CPP-SDK::Discovery ALIAS cpp-client-ydb_discovery)
