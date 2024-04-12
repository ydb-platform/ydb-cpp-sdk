add_library(client-ydb_types-fatal_error_handlers)

target_link_libraries(client-ydb_types-fatal_error_handlers PUBLIC
  yutil
  client-ydb_types-exceptions
)

target_sources(client-ydb_types-fatal_error_handlers PRIVATE
<<<<<<<< HEAD:src/client/types/fatal_error_handlers/CMakeLists.txt
<<<<<<< HEAD:src/client/types/fatal_error_handlers/CMakeLists.txt
  handlers.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/fatal_error_handlers/handlers.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/fatal_error_handlers/CMakeLists.txt
========
  handlers.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/fatal_error_handlers/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_types/fatal_error_handlers/CMakeLists.txt
========
  handlers.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/fatal_error_handlers/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


