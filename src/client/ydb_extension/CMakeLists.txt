add_library(cpp-client-ydb_extension)

target_link_libraries(cpp-client-ydb_extension PUBLIC
  yutil
  cpp-monlib-metrics
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_extension PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/ydb_extension/CMakeLists.txt
  extension.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_extension/extension.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_extension/CMakeLists.txt
=======
  extension.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_extension/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_library(YDB-CPP-SDK::Extension ALIAS cpp-client-ydb_extension)
