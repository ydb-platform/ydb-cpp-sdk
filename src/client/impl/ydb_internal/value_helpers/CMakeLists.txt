add_library(impl-ydb_internal-value_helpers)

target_link_libraries(impl-ydb_internal-value_helpers PUBLIC
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
)

target_sources(impl-ydb_internal-value_helpers PRIVATE
<<<<<<< HEAD:src/client/impl/ydb_internal/value_helpers/CMakeLists.txt
  helpers.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/value_helpers/helpers.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/value_helpers/CMakeLists.txt
)
