add_library(client-ydb_types-operation)

target_link_libraries(client-ydb_types-operation PUBLIC
  yutil
  protobuf::libprotobuf
  cpp-threading-future
  library-operation_id
  cpp-client-ydb_types
)

target_sources(client-ydb_types-operation PRIVATE
<<<<<<<< HEAD:src/client/types/operation/CMakeLists.txt
<<<<<<< HEAD:src/client/types/operation/CMakeLists.txt
  operation.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/operation/operation.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/operation/CMakeLists.txt
========
  operation.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/operation/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/operation/CMakeLists.txt
========
  operation.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/operation/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


