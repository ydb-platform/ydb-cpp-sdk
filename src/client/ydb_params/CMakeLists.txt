add_library(cpp-client-ydb_params)

target_link_libraries(cpp-client-ydb_params PUBLIC
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
  cpp-client-ydb_value
)

target_sources(cpp-client-ydb_params PRIVATE
<<<<<<<< HEAD:src/client/params/CMakeLists.txt
<<<<<<< HEAD:src/client/params/CMakeLists.txt
  params.cpp
  impl.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_params/params.cpp
  ${YDB_SDK_SOURCE_DIR}/client/ydb_params/impl.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_params/CMakeLists.txt
========
  params.cpp
  impl.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_params/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_params/CMakeLists.txt
========
  params.cpp
  impl.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_params/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::Params ALIAS cpp-client-ydb_params)
