add_library(impl-ydb_internal-logger)

target_link_libraries(impl-ydb_internal-logger PUBLIC
  yutil
  library-cpp-logger
)

target_sources(impl-ydb_internal-logger PRIVATE
<<<<<<< HEAD:src/client/impl/ydb_internal/logger/CMakeLists.txt
  log.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/logger/log.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/logger/CMakeLists.txt
)


