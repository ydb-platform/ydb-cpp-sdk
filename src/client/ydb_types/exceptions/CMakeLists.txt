add_library(client-ydb_types-exceptions)

target_link_libraries(client-ydb_types-exceptions PUBLIC
  yutil
)

target_sources(client-ydb_types-exceptions PRIVATE
<<<<<<<< HEAD:src/client/types/exceptions/CMakeLists.txt
<<<<<<< HEAD:src/client/types/exceptions/CMakeLists.txt
  exceptions.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/exceptions/exceptions.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/exceptions/CMakeLists.txt
========
  exceptions.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/exceptions/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/exceptions/CMakeLists.txt
========
  exceptions.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/exceptions/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


