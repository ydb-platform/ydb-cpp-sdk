add_library(impl-ydb_internal-logger)

target_link_libraries(impl-ydb_internal-logger PUBLIC
  yutil
  library-cpp-logger
)

target_sources(impl-ydb_internal-logger PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/impl/ydb_internal/logger/CMakeLists.txt
  log.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/logger/log.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/logger/CMakeLists.txt
=======
  log.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/logger/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  log.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  log.cpp
>>>>>>> origin/main
)


